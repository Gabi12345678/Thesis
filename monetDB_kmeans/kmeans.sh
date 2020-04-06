python3 kmeans_generate_udf.py
echo "Running UDF"
mclient -p54321 -d mydb udf.sql
echo "Removing files"
