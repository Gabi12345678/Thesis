package com.mcobject.extremedb;

public class TransIterator {

    public abstract static class Iterator {
        long handle;

        public void close() {
            if (handle != 0) {
                closeIterator(handle);
                handle = 0;
            }
        }
    }

    public static class JsonConverter extends Iterator {
        private java.io.OutputStream os;

        public JsonConverter(java.io.OutputStream os, boolean compact, boolean ignoreStreamErrors) {
            this.os = os;
            handle = createJsonConverter(os, compact, ignoreStreamErrors);
        }

        public JsonConverter(java.io.OutputStream os) {
            this.os = os;
            handle = createJsonConverter(os, true, false);
        }

        protected void finalize() {
            close();
        }

    }

    private static native void closeIterator(long handle);
    private static native long createJsonConverter(java.io.OutputStream os, boolean compact, boolean ignoreStreamErrors);
}
