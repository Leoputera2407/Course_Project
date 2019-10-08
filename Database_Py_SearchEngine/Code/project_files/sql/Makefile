# queries:
queries/exp_and_query.sql: queries/exp_header.sql queries/and_query.sql
	cat queries/exp_header.sql queries/and_query.sql > queries/exp_and_query.sql
queries/exp_new_and_query.sql: queries/exp_header.sql queries/new_and_query.sql
	cat queries/exp_header.sql queries/new_and_query.sql > queries/exp_new_and_query.sql
queries/exp_or_query.sql: queries/exp_header.sql queries/or_query.sql
	cat queries/exp_header.sql queries/or_query.sql > queries/exp_or_query.sql

# analyze query plans:
and_plan: queries/exp_and_query.sql
	psql -qAt -f queries/exp_and_query.sql  > and_plan
or_plan: queries/exp_or_query.sql
	psql -qAt -f queries/exp_or_query.sql  > or_plan
new_and_plan: queries/exp_new_and_query.sql
	psql -qAt -f queries/exp_new_and_query.sql  > new_and_plan

clean:
	rm -rf queries/exp_*query.sql *_plan 404997407 504971751 dist-folder P1A.zip || exit 0

dist: 404997407 504971751

404997407: dist-folder
	cp -r dist-folder 404997407
504971751: dist-folder
	cp -r dist-folder 504971751

dist-folder: schema.sql load.sql TEAM.txt README.txt
	rm -rf dist-folder; mkdir dist-folder; cp schema.sql load.sql TEAM.txt README.txt dist-folder/

