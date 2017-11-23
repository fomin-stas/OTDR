#!/bin/bash
dbname="optic_db"
username="postgres"

psql $dbname $username << EOF
SELECT
        TABLE_NAME,
        pg_size_pretty(pg_table_size(TABLE_NAME)) AS table_size,
        pg_size_pretty(pg_indexes_size(TABLE_NAME)) AS indexes_size,
        pg_size_pretty(pg_total_relation_size(TABLE_NAME)) AS total_size
    FROM (
        SELECT ('"' || table_schema || '"."' || TABLE_NAME || '"') AS TABLE_NAME
        FROM information_schema.tables
	WHERE table_schema = 'public'
    ) AS all_tables
    ORDER BY total_size DESC

EOF
