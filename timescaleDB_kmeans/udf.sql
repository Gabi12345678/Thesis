CREATE OR REPLACE FUNCTION distance(A datapoint, B datapoint) RETURNS double precision  AS $$
        DECLARE
                result double precision;
        BEGIN
                SELECT INTO result sum((CAST(dim_A.value as double precision) - CAST(dim_B.value as double precision)) ^ 2)
                        FROM json_each_text(row_to_json(A)) as dim_A, json_each_text(row_to_json(B)) as dim_B
                        WHERE dim_A.key = dim_B.key
                          AND dim_A.key LIKE 'd%'
                        LIMIT 1;
                RETURN result;
        END;
$$ LANGUAGE PLPGSQL;

CREATE OR REPLACE FUNCTION closest_cluster(A datapoint) RETURNS NUMERIC AS $$ SELECT cluster_id FROM clusters ORDER BY distance(A, clusters) LIMIT 1; $$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION update_dimension(d TEXT) RETURNS void AS $$
        DECLARE
                sql TEXT;
        BEGIN
                sql = FORMAT('UPDATE clusters SET %s = (SELECT AVG(%s) FROM datapoints WHERE datapoints.cluster_id = clusters.cluster_id);', d, d);
                EXECUTE sql;
        END;
$$ LANGUAGE PLPGSQL;

CREATE OR REPLACE FUNCTION get_dimensions() RETURNS TABLE (dimension TEXT) AS $$
        DECLARE
                A datapoint;
        BEGIN
                SELECT INTO A * FROM datapoints LIMIT 1;
                RETURN QUERY SELECT t.key FROM json_each_text(row_to_json(A)) as t WHERE t.key LIKE 'd%';
        END;
$$ LANGUAGE PLPGSQL;

CREATE OR REPLACE FUNCTION kmeans() RETURNS void AS $$
        DECLARE
                n BIGINT;
                initial_time TIMESTAMP WITH TIME ZONE;
                final_time TIMESTAMP WITH TIME ZONE;
        BEGIN
                initial_time := clock_timestamp();
                FOR counter IN 1..20 LOOP
                        RAISE NOTICE 'Iteration: %', counter;
                        UPDATE datapoints SET cluster_id = closest_cluster(datapoints);
                        PERFORM update_dimension(t.dimension) FROM get_dimensions() as t;
                END LOOP;
                final_time := clock_timestamp();
                RAISE NOTICE 'Initial time: %', initial_time;
                RAISE NOTICE 'Final time: %', final_time;
                RAISE NOTICE 'Total time: %', final_time - initial_time;
        END;
$$ LANGUAGE PLPGSQL;

SELECT kmeans();