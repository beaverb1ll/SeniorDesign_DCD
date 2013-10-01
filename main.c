
//
// SELECT Ing0, Ing1, Ing2, Ing3, Ing4, Ing5, orderTime, pickupTime FROM orderTable WHERE expired=false"
//
/*

    This program will take care of de-Allocating any drinks that have expired and have not been picked up.
    For simplicity, this program will assume that any drink that has been picked up will already be marked as expired.

    This wil do two different things.
        1) mark any expired drinks as expired.
        2) restore the ingredients from expired drinks


// SQL Schema-> ingredTable:
    id,
    ingred0,
    ingred1,
    ingred2,
    ingred3,
    ingred4,
    ingred5




*/

#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <mysql/mysql.h>


#define TRUE 1
#define FALSE 0

#define BARCODE_LENGTH 50
#define NUM_INGREDIENTS 6
#define MAX_SECONDS_RESERVED 600 // 10 minutes

struct settings {
    char dbName[100];
    char dbUsername[100];
    char dbPasswd[100];
};

int unreserveIngred(MYSQL *sql_con, MYSQL_ROW order_row);
MYSQL* openSQL(const char *db_username, const char *db_passwd, const char *db_name);
int cleanupDB(MYSQL *sql_connection);
struct settings* parseArgs(int argc, char const *argv[]);

int main(int argc, char const *argv[])
{

    MYSQL *con_SQL;
    struct settings *currentSettings;

    openlog("DCD", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Daemon Started.\n");

    // parse args
    currentSettings = parseArgs(argc, argv);
    syslog(LOG_INFO, "Finished parsing input arguments.");

    // open sql
    con_SQL = openSQL(currentSettings->dbUsername, currentSettings->dbPasswd, currentSettings->dbName);

    // query and edit db.
    return cleanupDB(con_SQL);
}

MYSQL* openSQL(const char *db_username, const char *db_passwd, const char *db_name)
{

	MYSQL *con = mysql_init(NULL);

	if (con == NULL)
	{
    		syslog(LOG_INFO, "Error in openSQL: %s  Exiting...\n", mysql_error(con));
     		exit(1);
	}

  	if (mysql_real_connect(con, "127.0.0.1", db_username, db_passwd, db_name, 0, NULL, 0) == NULL)
	{
    		syslog(LOG_INFO, "Error in openSQL: %s  Exiting...\n", mysql_error(con));
    		mysql_close(con);
  		exit(1);
  	}
  	return  con;
}

struct settings* parseArgs(int argc, char const *argv[])
{
    struct settings* allSettings;
    int temp, opt;

    allSettings = malloc(sizeof(struct settings));
    if (allSettings == NULL)
    {
        syslog(LOG_INFO, "Unable to create settings struct. Exiting...");
        exit(1);
    }

    // strcpy(allSettings->dbName, "orderTable");
    // strcpy(allSettings->dbUsername, "root");
    // strcpy(allSettings->dbPasswd, "password");
    // strcpy(allSettings->cbDevice, "/dev/ttyS0");
    // strcpy(allSettings->barcodeDevice, "/dev/ttyS1");
    // allSettings->cbBaud = 17;
    // allSettings->barcodeBaud = 17;
//  -u <username> -p <password> -d <databaseName> -c <control board tty device name> -b <barcode tty device name> -r <baudRate> -s <baudRate>

    while ((opt = getopt(argc, argv, "u:p:d:c:b:r:s:")) != -1)
    {
        switch(opt)
        {
            case 'u': // username
                strcpy(allSettings->dbUsername, optarg);
                break;

            case 'p': // password
                strcpy(allSettings->dbPasswd, optarg);
                break;

            case 'd': // dbName
            strcpy(allSettings->dbName, optarg);
                break;
        case '?':
            syslog(LOG_INFO, "Invalid startup argument: %c. Exiting...", optopt);
            exit(1);
            break;
        }
    }




    return allSettings;
}

int cleanupDB(MYSQL *sql_connection)
{
    int num_rows, i;
    MYSQL_ROW row;
    MYSQL_RES *result;
    int *ingred;
    time_t currentTime, orderTime;
    double timePassed;

    char *baseUpdateExpired = "UPDATE orderTable SET expired='true' WHERE orderID='";
    char *fetchExpired = "SELECT Ing0, Ing1, Ing2, Ing3, Ing4, Ing5, orderID, orderTime FROM orderTable WHERE expired='false'";
    char queryString[300];

    if (mysql_query(sql_connection, fetchExpired)) {
        syslog(LOG_INFO, "Unable to query SQL to find expired orders");
        return 1;
    }

    result = mysql_store_result(sql_connection);
    if (result == NULL)
    {
      syslog(LOG_INFO, "Unable to get expired results.");
      return 1;
    }

    num_rows = mysql_num_rows(result);
    if (num_rows < 0)
    {
      syslog(LOG_INFO, "Error: Invalid number of rows: %d", num_rows);
      return 1;
    }


    while(row = mysql_fetch_row(result)) // row pointer in the result set
        {

            // if here, the row has not yet been picked up, and is not marked expired.
            // so we need to see if drink is expired by comparing ordertime to currentTime
			currentTime = time(NULL);
            orderTime = atoi(row[7]);
            syslog(LOG_INFO, "orderTime: %d", orderTime);

            timePassed = difftime(currentTime, orderTime);
            syslog(LOG_INFO, "timePassed: %lf", timePassed);

            if (timePassed < 1)
            {
                syslog(LOG_INFO, "Invalid time difference of: %lf. skipping...", timePassed);
                return 1;
            } else if(timePassed > MAX_SECONDS_RESERVED) // update sql to true here
            {
                // create query string
                strcpy(queryString, baseUpdateExpired);
                strcat(queryString, row[6]);
                strcat(queryString, "'");

                // update sql
                syslog(LOG_INFO, "Reservation time expired. Setting barcode %s to expired...", row[6]);

                if (mysql_query(sql_connection, queryString))
                {
                    syslog(LOG_INFO, "Unable to update SQL");
                } else
                {
                    syslog(LOG_INFO, "Updated SQL.");
                    // update ingredient table here
                    if(unreserveIngred(sql_connection, row))
                    {
                        syslog(LOG_INFO, "Error withdrawing reservation for barcode: %s", row[6]);
                    }
                }
            } else
            {
                syslog(LOG_INFO, "order not expired. Skipping...");
            }
        }
        mysql_free_result(result);
        syslog(LOG_INFO, "Done updating expired orders");
        return 0;
    }

int unreserveIngred(MYSQL *sql_con, MYSQL_ROW order_row)
{
    MYSQL_ROW row;
    MYSQL_RES *result;
    int num_rows, i;
    int ingredLevelArray[NUM_INGREDIENTS];
    int orderIngredArray[NUM_INGREDIENTS];
    char *ingredLevelQuery = "SELECT ing0, ing1, ing2, ing3, ing4, ing5 FROM orderTable WHERE orderID=\"0\"";
    char queryString[300];

    // get current amounts from sql_con_ingred
    if (mysql_query(sql_con, ingredLevelQuery)) {
        syslog(LOG_INFO, "Unable to query SQL to find expired orders");
        return 1;
    }

    result = mysql_store_result(sql_con);
    if (result == NULL)
    {
      syslog(LOG_INFO, "Unable to get expired results.");
      return 1;
    }

    num_rows = mysql_num_rows(result);
    if (num_rows < 0)
    {
      syslog(LOG_INFO, "Error: Invalid number of rows: %d", num_rows);
      return 1;
    } else if (num_rows != 1)
    {
        syslog(LOG_INFO, "Error: Too many ingredient level rows returned.");
        return 1;
    }
    row = mysql_fetch_row(result);
    // convert row ingreds to integers
    for (i = 0; i < NUM_INGREDIENTS; ++i)
    {
        // convert both to ints
        ingredLevelArray[i] = atoi(row[i]);
        orderIngredArray[i] = atoi(order_row[i]);
        // perform math to update
        ingredLevelArray[i] += orderIngredArray[i];
    }
    // construct query string
    sprintf(queryString, "UPDATE orderTable SET ing0=%d, ing1=%d, ing2=%d, ing3=%d, ing4=%d, ing5=%d  WHERE orderID=\"0\"", ingredLevelArray[0], ingredLevelArray[1], ingredLevelArray[2], ingredLevelArray[3], ingredLevelArray[4], ingredLevelArray[5] );

    // query
    if (mysql_query(sql_con, queryString))
    {
        syslog(LOG_INFO, "Error when updating Ingred values");
    }
    // return without error
    return 0;
}
