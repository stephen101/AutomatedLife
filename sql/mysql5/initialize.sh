DATABASE_NAME="enron_index4";
USER="aaron";

echo $DATABASE_NAME;
echo $USER;

mysql -uroot -e "DROP DATABASE IF EXISTS ${DATABASE_NAME};
				 CREATE DATABASE ${DATABASE_NAME}; 
				 GRANT ALL ON ${DATABASE_NAME}.* TO '$USER'@'localhost';";

cat tables.sql triggers.sql procedures.sql | mysql -uroot $DATABASE_NAME;

