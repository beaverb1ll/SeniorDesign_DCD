
// 
// SELECT Ing0, Ing1, Ing2, Ing3, Ing4, Ing5, orderTime, pickupTime FROM orderTable WHERE expired=false"
//
/*

    This program will take care of de-Allocating any drinks that have expired and have not been picked up.
    For simplicity, this program will assume that any drink that has been picked up will already be marked as expired.

    This wil do two different things.
        1) mark any expired drinks as expired.
        2) restore the ingredients from expired drinks




*/

int main(int argc, char const *argv[])
{
    // parse args

    // open sql

    // query and edit db.


    return 0;
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

int* getIngredFromSQL(MYSQL *sql_con, const char *query)
{
  int num_rows, i;
  MYSQL_ROW row;
  MYSQL_RES *result;
  int *ingred;
  time_t currentTime, orderTime;
  double timePassed;
//  int orderTime;

  if (mysql_query(sql_con, query)) {
      syslog(LOG_INFO, "Unable to query SQL with string: %s", query);
      return NULL;
  }

  result = mysql_store_result(sql_con);

    if (result == NULL)
    {
      syslog(LOG_INFO, "Unable to get result from SQL query: %s", query);
      return NULL;
  }

    num_rows = mysql_num_rows(result);


    if (num_rows < 0)
    {
      syslog(LOG_INFO, "Error: Invalid number of rows: %d", num_rows);
      return NULL;
    }


    while(row = mysql_fetch_row(result)) // row pointer in the result set
        {

            // if here, the row has not yet been picked up, and is not marked expired.
            // so we need to see if drink is expired by comparing ordertime to currentTime


            // if difference is greater than expire_time, then:
            // mark orderID as expired=true


            // add ingredients back to ingredientTable

        }
        //     for(int ii=0; ii < numFields; ii++)
        //     {
        //         printf("%s\t", mysqlRow[ii] ? mysqlRow[ii] : "NULL");  // Not NULL then print
        //     }
        //     printf("\n");
        // }








    // ========= DEBUG =====================
    int numFields = mysql_num_fields(result);
    int ii;
    for( ii=0; ii < numFields; ii++)
    {
      syslog(LOG_INFO, "%d : %s",ii, row[ii] ? row[ii] : "NULL");  // Not NULL then print
    }
    // ======== END DEBUG ==================


  // verify drink has not been picked up yet.
  if(!strcmp("true", row[NUM_INGREDIENTS]))
  {
    syslog(LOG_INFO, "Drink already picked up");
    return NULL;
  }
  syslog(LOG_INFO, "Drink has not been picked up");

  // verify time hasn't expired
  currentTime = time(NULL);
  syslog(LOG_INFO, "currentTime: %d", currentTime);

  char testString[60];
  strcpy(testString, row[7]);
  syslog(LOG_INFO, "testString: %s", testString);
  orderTime = atoi(testString);

  syslog(LOG_INFO, "orderTime: %d", orderTime);

  timePassed = difftime(currentTime, orderTime);
  syslog(LOG_INFO, "timePassed: %lf", timePassed);

  if (timePassed < 1)
  {
    syslog(LOG_INFO, "Invalid time difference of: %lf. skipping...", timePassed);
    return NULL;
  } else if(timePassed > MAX_SECONDS_RESERVED)
  {
    syslog(LOG_INFO, "Reservation time expired. skipping...");
    return NULL;
  }
  syslog(LOG_INFO, "order not expired. will dispense.");


  ingred = (int*)malloc(sizeof(int) * NUM_INGREDIENTS);
  // store ingredients from row data
    for (i = 0; i < NUM_INGREDIENTS; i++)
    {
      ingred[i] = atoi(row[i]);;
    }

    mysql_free_result(result);
    return ingred;
}