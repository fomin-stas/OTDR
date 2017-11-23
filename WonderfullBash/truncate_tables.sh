#!/bin/sh

dbname="optic_db"
username="postgres"
psql $dbname $username << EOF
truncate alfa_compare, dots;


EOF

