#ifndef MCORESTAPI_H_
#define MCORESTAPI_H_

#include "mcorest.h"

#include "ws/mcohttp.h"


/*
 * Size of the internal buffer of the JSON reader.
 */
#define MCOREST_API_JSON_READER_BUFFER_SIZE 256

/*
 * Maximum length of a numeric value.
 */
#define MCOREST_API_JSON_MAX_NUMERIC_LEN 64


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Stream read function.
 * @param [in] ctx Context pointer, as specified in mcorest_api_stream_t.
 * @param [in] buf Buffer to read data into.
 * @param [in] len Size of the buffer.
 * @retval < 0 on error, 0 on EOF, or actual number of bytes read.
 */
typedef int (* mcorest_api_stream_read_fn)(void *ctx, char *buf, int len);

/*
 * Stream write function.
 * @param [in] ctx Context pointer, as specified in mcorest_api_stream_t.
 * @param [in] buf Buffer to write.
 * @param [in] len Size of the buffer.
 * @retval < 0 on error, 0 on EOF, or actual number of bytes written.
 */
typedef int (* mcorest_api_stream_write_fn)(void *ctx, const char *buf, int len);

/*
 * Stream structure.
 */
typedef struct mcorest_api_stream_t_
{
    void *ctx;  /* User-defined context that will be passed to read_fn and write_fn. */
    mcorest_api_stream_read_fn read_fn;  /* Stream read function. */
    mcorest_api_stream_write_fn write_fn;  /* Stream write function. */
} mcorest_api_stream_t;


/*
 * JSON writer flags.
 */
typedef enum MCOREST_API_JSON_WR_FLAGS_E_
{
    MCOREST_API_JSON_WR_FLAG_NONE = 0x00,
    MCOREST_API_JSON_WR_FLAG_STRING_NUMERICS = 0x01  /* Send JSON numerics as strings. */
} MCOREST_API_JSON_WR_FLAGS;

/*
 * JSON writer context structure.
 */
typedef struct mcorest_api_json_write_ctx_t_
{
    mcorest_api_stream_t *stream;  /* Stream to write JSON data into. */
    int indent;  /* Indentation level for pretty-printing. */

    /* The following fields are for the JSON encoder's internal use. */
    int first_entry;  /* Non-zero if the next key is the first in the object/array. */
    int key_allowed;  /* Non-zero if a key is allowed to be written. */
    int key_was_put;  /* Non-zero if a key has just been written. */
    int obj_level;  /* Object nesting level. */
    int arr_level;  /* Array nesting level. */

    mco_uint2 flags;  /* A combination of MCOREST_API_JSON_WR_FLAGS values. */
} mcorest_api_json_write_ctx_t;


/*
 * JSON reader flags.
 */
typedef enum MCOREST_API_JSON_RD_FLAGS_E_
{
    MCOREST_API_JSON_RD_FLAG_NONE = 0x00,
    MCOREST_API_JSON_RD_FLAG_STRING_NUMERICS = 0x01  /* Allow JSON numerics to be read from strings. */
} MCOREST_API_JSON_RD_FLAGS;

/*
 * JSON reader context structure.
 */
typedef struct mcorest_api_json_read_ctx_t_
{
    mcorest_api_stream_t *stream;  /* Stream to read JSON data from. */

    /* The following fields are for the JSON reader's internal use. */
    int stream_err;  /* Stream error flag. */
    int stream_end;  /* Stream end flag. */
    char buf[MCOREST_API_JSON_READER_BUFFER_SIZE];  /* Stream input buffer. */
    int buf_len;  /* Number of bytes available in the input buffer. */
    const char *buf_p;  /* Current buffer position pointer. */

    int obj_level;  /* Object nesting level. */
    int arr_level;  /* Array nesting level. */
    int read_escaped;  /* Escaped character read flag. */

    mco_uint2 flags;  /* A combination of MCOREST_API_JSON_RD_FLAGS values. */
} mcorest_api_json_read_ctx_t;

/*
 * JSON reader token types.
 */
typedef enum MCOREST_API_JSON_TKN_TYPE_E_
{
    MCOREST_API_JSON_TKN_UNKNOWN = 0,  /* Unknown token. */
    MCOREST_API_JSON_TKN_OBJECT_BGN,  /* Object start ('{'). */
    MCOREST_API_JSON_TKN_OBJECT_END,  /* Object end ('}'). */
    MCOREST_API_JSON_TKN_ARRAY_BGN,  /* Array start ('['). */
    MCOREST_API_JSON_TKN_ARRAY_END,  /* Array end (']'). */
    MCOREST_API_JSON_TKN_STRING,  /* Quoted string. */
    MCOREST_API_JSON_TKN_NUMBER,  /* Number (integer or real). */
    MCOREST_API_JSON_TKN_TRUE,  /* "true" literal. */
    MCOREST_API_JSON_TKN_FALSE,  /* "false" literal. */
    MCOREST_API_JSON_TKN_NULL  /* "null" literal. */
} MCOREST_API_JSON_TKN_TYPE;


/*
 * Unique request identifier.
 */
typedef mco_uint8 mcorest_api_request_id;


/*
 * HTTP request structure.
 */
typedef struct mcorest_api_request_t_
{
    mcohs_request_h hs_req;  /* Underlying MCO EWS HTTP request. */

    mcorest_api_stream_t stream;  /* I/O stream connected to this request. */

    mcorest_api_json_read_ctx_t jrctx;  /* Default JSON reader context for this request. */
    mcorest_api_json_write_ctx_t jwctx;  /* Default JSON writer context for this request. */

    mcorest_api_request_id id;  /* Unique request identifier. */
} mcorest_api_request_t;


/*
 * Web service request handler routine.
 * If this routine finishes successfully, it is expected to return MCO_S_OK.
 * In case of a failure, the handler may return an MCO error code. If the HTTP
 * request has not been answered yet (i.e. no data has been sent),
 * the Web server will return HTTP status code 500 to the caller, as well as
 * error details in JSON format. The web service may use
 * the mcorest_api_failure() function to provide additional details
 * to the client.
 * If the handler routine sends data (e.g. HTTP status and headers) to
 * the client and then returns an error code, the connection is aborted by the
 * Web server. Any buffered data is discarded.
 * Alternatively, the handler may opt to return the corresponding HTTP status
 * to the client and write any associated information itself. It should return
 * MCO_S_OK afterwards.
 * @param [in] ctx Context pointer, as specified by the mcorest_api_register_service call.
 * @param [in] request Request structure.
 * @param [in] path_suffix Any remaining characters of the path: child path
 * elements, query parameters, etc.
 * @retval MCO_S_OK on success, error code otherwise.
 */
typedef MCO_RET (* mcorest_api_handler_fn)(void *ctx, mcorest_api_request_t *request, const char *path_suffix);

/*
 * Web service request cancellation handler routine.
 * If this callback is specified when registering a service, then, if the
 * client closes the network connection, this function will be called to notify
 * the service.
 * This functionality is only available when mcorest is built with
 * both multithreading and dynamic memory enabled.
 * @param [in] req_id Identifier of the cancelled request.
 */
typedef void (* mcorest_api_cancel_handler_fn)(mcorest_api_request_id req_id);

/*
 * Registers REST service at given path. The registration order matters:
 * the paths should be registered from the most to the least specific
 * (e.g. "/base/child" should come before "/base").
 * @param [in] rest Server instance handle.
 * @param [in] path Service's URL path.
 * @param [in] h Request handler routine.
 * @param [in] ch Request cancellation handler routine. This parameter can be
 * NULL if the service does not support request cancellation.
 * @param [in] ctx Context pointer to be passed to the handler.
 * @param [in] allow_partial_match If not 0, the handler will be passed requests
 * for both path and its children (e.g. handler for "/base" will also receive
 * "/base/child" requests). Otherwise, only exact matches will be accepted
 * (e.g. "/base", "/base/" or "/base?query").
 * @retval MCO_S_OK Success.
 * @retval MCO_E_REST_INUSE A service has already been registered with the path.
 * @retval MCO_E_REST_* error code on failure.
 */
MCOREST_API MCO_RET mcorest_api_register_service(mcorest_h rest, const char *path,
    mcorest_api_handler_fn h, mcorest_api_cancel_handler_fn ch,
    void *ctx, int allow_partial_match);


/*
 * Sends HTTP OK response. This function should be called explicitly prior to
 * sending any JSON data.
 * @param [in] req Request object to write response into.
 */
MCOREST_API void mcorest_api_status_ok(mcorest_api_request_t *req);

/*
 * Sends HTTP "bad request" response. No further content should be sent.
 * @param [in] req Request object to write response into.
 * @param [in] info Optional additional information (plain text).
 */
MCOREST_API void mcorest_api_status_bad_request(mcorest_api_request_t *req, const char *info);

/*
 * Sends HTTP "internal error" response. No further content should be sent.
 * @param [in] req Request object to write response into.
 * @param [in] info Optional additional information (plain text).
 */
MCOREST_API void mcorest_api_status_internal_error(mcorest_api_request_t *req, const char *info);

/*
 * Sends HTTP "unauthorized" response. No further content should be sent.
 * @param [in] req Request object to write response into.
 * @param [in] auth_realm HTTP basic authentication realm.
 */
void mcorest_api_status_unauthorized(mcorest_api_request_t *req, const char *auth_realm);

/*
 * Sends HTTP "forbidden" response. No further content should be sent.
 * @param [in] req Request object to write response into.
 */
void mcorest_api_status_forbidden(mcorest_api_request_t *req);

/*
 * Sends HTTP "not found" response. No further content should be sent.
 * @param [in] req Request object to write response into.
 */
MCOREST_API void mcorest_api_status_not_found(mcorest_api_request_t *req);

/*
 * Initialises stream with an HTTP request.
 * @param [in] req Request to connect stream to.
 * @param [out] s Stream bound to the request.
 */
MCOREST_API void mcorest_api_stream_with_request(mcorest_api_request_t *req, mcorest_api_stream_t *s);

/*
 * Returns HTTP request object associated with the stream.
 * @param [in] s Pointer to the stream.
 * @retval Pointer to the request object.
 */
MCOREST_API mcorest_api_request_t *mcorest_api_request_from_stream(mcorest_api_stream_t *s);

/*
 * Records details of a handler failure. This information will be returned to
 * the REST API caller.
 * No data should be written into the request after this function is called.
 * Subsequent calls are ignored.
 * The is_internal flag indicates that the failure is internal to the service,
 * as opposed to being caused by an invalid request. Internal errors are
 * reported to the REST API client using HTTP status code 500; external ones
 * are reported using HTTP status code 400.
 * @param [in] req Failed HTTP request.
 * @param [in] file Name of the file where the failure was detected.
 * @param [in] line Line of code where the failure was detected.
 * @param [in] is_internal Set to non-zero to indicate that the error is
 * internal to the service and was not caused by an invalid request.
 */
MCOREST_API void mcorest_api_failure(mcorest_api_request_t *req, const char *file, int line, int is_internal);

/*
 * Resets the previously set failure details associated with the HTTP request
 * object.
 * @param [in] req HTTP request.
 */
MCOREST_API void mcorest_api_failure_clear(mcorest_api_request_t *req);


/*
 * Initialises the JSON writer context object.
 * @param [in] s JSON stream to write data to.
 * @param [in,out] ctx Pointer to the JSON context structure to initialize.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_init(mcorest_api_stream_t *s, mcorest_api_json_write_ctx_t *ctx);

/*
 * Starts writing JSON to a stream. Opens the top-level JSON object.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_start(mcorest_api_json_write_ctx_t *ctx);

/*
 * Writes a JSON key.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] key Key name to write.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_key(mcorest_api_json_write_ctx_t *ctx, const char *key);

/*
 * Starts a JSON object, outputting the opening brace.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_object_begin(mcorest_api_json_write_ctx_t *ctx);

/*
 * Ends the JSON object, outputting the closing brace.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_object_end(mcorest_api_json_write_ctx_t *ctx);

/*
 * Starts a JSON array, outputting the opening bracket.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_array_begin(mcorest_api_json_write_ctx_t *ctx);

/*
 * Ends the JSON array, outputting the closing bracket.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_array_end(mcorest_api_json_write_ctx_t *ctx);

/*
 * Writes a boolean value.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] b Value to write; interpreted as "true" if non-zero, "false" otherwise.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_bool(mcorest_api_json_write_ctx_t *ctx, int b);

/*
 * Writes a long integer value.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] l Value to write.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_long(mcorest_api_json_write_ctx_t *ctx, long l);

/*
 * Writes an unsigned long integer value.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] u Value to write.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_ulong(mcorest_api_json_write_ctx_t *ctx, unsigned long u);

/*
 * Writes a 64-bit integer value.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] l Value to write.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_i64(mcorest_api_json_write_ctx_t *ctx, mco_int8 i);

/*
 * Writes an unsigned 64-bit integer value.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] l Value to write.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_u64(mcorest_api_json_write_ctx_t *ctx, mco_uint8 u);

/*
 * Writes a floating-point value.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] d Value to write.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_double(mcorest_api_json_write_ctx_t *ctx, double d);

/*
 * Starts writing a string value, outputting the opening quote.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_string_begin(mcorest_api_json_write_ctx_t *ctx);

/*
 * Writes a fragment of the string value.
 * If len is zero, the string is written until the terminating zero
 * is reached.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] s String fragment to write.
 * @param [in] len Length of the string fragment.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_string_part(mcorest_api_json_write_ctx_t *ctx, const char *s, size_t len);

/*
 * Writes a fragment of a hexadecimal representation of binary data.
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] b Data to write.
 * @param [in] len Length of the data array.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_hex_string_part(mcorest_api_json_write_ctx_t *ctx, const void *b, size_t len);

/*
 * Ends the string value, outputting the closing quote.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_string_end(mcorest_api_json_write_ctx_t *ctx);

/*
 * Writes a string. Escapes double quotes and backslashes.
 * If the string is NULL, writes null.
 * This is a convenience wrapper for mcorest_api_json_write_string_begin(),
 * mcorest_api_json_write_string_part() and mcorest_api_json_write_string_end().
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] s String to write.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_string(mcorest_api_json_write_ctx_t *ctx, const char *s);

/*
 * Writes binary data converted to hexadecimal string representation.
 * If the pointer is NULL, writes null.
 * This is a convenience wrapper for mcorest_api_json_write_string_begin(),
 * mcorest_api_json_write_hex_string_part() and mcorest_api_json_write_string_end().
 * @param [in] ctx Pointer to the current JSON context.
 * @param [in] b Data to write.
 * @param [in] len Length of the data array.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_hex_string(mcorest_api_json_write_ctx_t *ctx, const void *b, size_t len);

/*
 * Writes null.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_null(mcorest_api_json_write_ctx_t *ctx);

/*
 * Closes the top-level JSON object and does sanity checks. No further writes
 * to this JSON context are allowed.
 * @param [in] ctx Pointer to the current JSON context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_write_finish(mcorest_api_json_write_ctx_t *ctx);


/*
 * Initialises the JSON reader context object.
 * @param [in] s JSON stream to read data from.
 * @param [in,out] ctx Pointer to the JSON context structure to initialize.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_init(mcorest_api_stream_t *s, mcorest_api_json_read_ctx_t *ctx);

/*
 * Starts reading JSON data from a stream.
 * @param [in] ctx Pointer to the current JSON reader context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_start(mcorest_api_json_read_ctx_t *ctx);

/*
 * Retrieves type of the next token in the JSON stream. If expected_type is not
 * MCOREST_API_JSON_TKN_UNKNOWN, and it does not match the next token
 * in the stream, MCO_E_REST_JSON error is returned, and actual type
 * of the token is returned through the "type" pointer.
 * Depending on the returned token type, one of the read functions
 * should be called to read the data:
 * - MCOREST_API_JSON_TKN_OBJECT_BGN: mcorest_api_json_read_object_begin()
 * - MCOREST_API_JSON_TKN_OBJECT_END: mcorest_api_json_read_object_end()
 * - MCOREST_API_JSON_TKN_ARRAY_BGN: mcorest_api_json_read_array_begin()
 * - MCOREST_API_JSON_TKN_ARRAY_END: mcorest_api_json_read_array_end()
 * - MCOREST_API_JSON_TKN_STRING: mcorest_api_json_read_string()
 * - MCOREST_API_JSON_TKN_NUMBER: mcorest_api_json_read_[numeric/long/i64/double]()
 * If the returned type indicates one of the literals (MCOREST_API_JSON_TKN_TRUE,
 * MCOREST_API_JSON_TKN_FALSE, or MCOREST_API_JSON_TKN_NULL), no additional
 * mcorest_api_json_read_*() calls are needed. The caller should proceed to
 * read the next token type.
 * Note that if the MCOREST_API_JSON_RD_FLAG_STRING_NUMERICS flag is set and
 * a double-quoted numeric value is encountered, the "type" argument will still
 * be set to MCOREST_API_JSON_TKN_STRING. The calling code is responsible for
 * inferring the proper type from the context.
 * @param [in] ctx Pointer to the current JSON reader context.
 * @param [in] expected_type Expected type of the next token.
 * @param [out] type Optional; actual type of the next token.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_next_token(mcorest_api_json_read_ctx_t *ctx,
    MCOREST_API_JSON_TKN_TYPE expected_type,
    MCOREST_API_JSON_TKN_TYPE *type);

/*
 * Begins reading a JSON object.
 * @param [in] ctx Pointer to the current JSON reader context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_object_begin(mcorest_api_json_read_ctx_t *ctx);

/*
 * Finishes reading the JSON object.
 * @param [in] ctx Pointer to the current JSON reader context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_object_end(mcorest_api_json_read_ctx_t *ctx);

/*
 * Begins reading a JSON array.
 * @param [in] ctx Pointer to the current JSON reader context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_array_begin(mcorest_api_json_read_ctx_t *ctx);

/*
 * Finishes reading the JSON array.
 * @param [in] ctx Pointer to the current JSON reader context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_array_end(mcorest_api_json_read_ctx_t *ctx);

/*
 * Reads quoted string from the input stream.
 * It is possible that the JSON string does not fit into the provided buffer.
 * In that case, the output value of "len" will be the same as input.
 * The following fragment(s) of the string can be read with subsequent calls
 * to this function with the "cont" flag set to non-zero value.
 * If the JSON string fits into the buffer, the output "len" value will _always_
 * be smaller than the input value. No subsequent calls to this function
 * should be made in this case.
 * The buffer is _never_ zero-terminated.
 * Both keys and string values in JSON have the same format. The JSON reader
 * does not track the JSON context, and thus cannot differentiate keys from
 * string values. The application should set the "as_key" flag to non-zero value
 * if it expects the string to be a key. An error will be returned if this flag
 * is not set but the string turns out to be a key (i.e. is followed by ':'),
 * or vice versa.
 * This function decodes JSON escaped characters, except for \uXXXX.
 * @param [in] ctx Pointer to the current JSON reader context.
 * @param [in] as_key Set to non-zero to treat the string value as JSON key.
 * @param [in] cont Set to non-zero in subsequent calls to this function
 * to retrieve consecutive fragments of the string.
 * @param [in] buf Buffer to read data into. This buffer is never zero-terminated.
 * @param [in,out] len On input, defines the size of the buffer. On output,
 * returns the actual number of characters written into the buffer.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_string(mcorest_api_json_read_ctx_t *ctx, int as_key, int cont, char *buf, int *len);

/*
 * Reads numeric value as string and does basic sanity checks.
 * The buffer is _always_ zero-terminated; the value is truncated if
 * it does not fit.
 * If the MCOREST_API_JSON_RD_FLAG_STRING_NUMERICS flag is set and the value
 * in the buffer is double-quoted, the quotes are stripped.
 * @param [in] ctx Pointer to the current JSON reader context.
 * @param [in] buf Buffer to read value into.
 * @param [in] len Size of the buffer.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_numeric(mcorest_api_json_read_ctx_t *ctx, char *buf, int len);

/*
 * Returns non-zero value if "str" contains integer number. Return value
 * is undefined if "str" does not contain a valid JSON numeric value.
 * @param [in] str Numeric JSON value.
 * @retval non-zero if "str" is integer, zero otherwise.
 */
MCOREST_API int mcorest_api_json_numeric_is_integer(const char *str);

/*
 * Converts numeric JSON value to long. If "str" does not contain an integer,
 * returns error.
 * @param [in] str Integer numeric JSON value.
 * @param [out] val Converted long value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_numeric_as_long(const char *str, long *val);

/*
 * Converts numeric JSON value to unsugned long. If "str" does not contain
 * an integer, returns error.
 * @param [in] str Integer numeric JSON value.
 * @param [out] val Converted unsigned long value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_numeric_as_ulong(const char *str, unsigned long *val);

/*
 * Converts numeric JSON value to a 64-bit integer. If "str" does not contain
 * an integer, returns error.
 * @param [in] str Integer numeric JSON value.
 * @param [out] val Converted 64-bit integer value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_numeric_as_i64(const char *str, mco_int8 *val);

/*
 * Converts numeric JSON value to unsigned 64-bit integer. If "str"
 * does not contain an integer, returns error.
 * @param [in] str Integer numeric JSON value.
 * @param [out] val Converted unsigned 64-bit integer value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_numeric_as_u64(const char *str, mco_uint8 *val);

/*
 * Converts numeric JSON value to double. If "str" does not contain
 * a numeric value, returns error.
 * @param [in] str Numeric JSON value.
 * @param [out] val Converted double value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_numeric_as_double(const char *str, double *val);

/*
 * Convenience wrapper for mcorest_api_json_read_numeric() and
 * mcorest_api_json_numeric_as_long().
 * @param [in] ctx Pointer to the current JSON reader context.
 * @param [out] val Converted long value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_long(mcorest_api_json_read_ctx_t *ctx, long *val);

/*
 * Convenience wrapper for mcorest_api_json_read_numeric() and
 * mcorest_api_json_numeric_as_ulong().
 * @param [in] ctx Pointer to the current JSON reader context.
 * @param [out] val Converted unsigned long value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_ulong(mcorest_api_json_read_ctx_t *ctx, unsigned long *val);

/*
 * Convenience wrapper for mcorest_api_json_read_numeric() and
 * mcorest_api_json_numeric_as_i64().
 * @param [in] ctx Pointer to the current JSON reader context.
 * @param [out] val Converted 64-bit integer value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_i64(mcorest_api_json_read_ctx_t *ctx, mco_int8 *val);

/*
 * Convenience wrapper for mcorest_api_json_read_numeric() and
 * mcorest_api_json_numeric_as_u64().
 * @param [in] ctx Pointer to the current JSON reader context.
 * @param [out] val Converted unsigned 64-bit integer value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_u64(mcorest_api_json_read_ctx_t *ctx, mco_uint8 *val);

/*
 * Convenience wrapper for mcorest_api_json_read_numeric() and
 * mcorest_api_json_numeric_as_double().
 * @param [in] ctx Pointer to the current JSON reader context.
 * @param [out] val Converted double value.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_double(mcorest_api_json_read_ctx_t *ctx, double *val);

/*
 * Finishes reading JSON data from the stream and does sanity checks. No further
 * reads are allowed for "ctx".
 * @param [in] ctx Pointer to the current JSON reader context.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_api_json_read_finish(mcorest_api_json_read_ctx_t *ctx);


#ifdef __cplusplus
}
#endif


#endif /* MCORESTAPI_H_ */
