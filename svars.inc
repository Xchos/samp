#if defined _SVARS_included
  #endinput
#endif

#include <a_samp>
#include <a_mysql>

#define _SVARS_included
#pragma library SVARS

#define MAX_SVAR_LENGTH                 128
#define MAX_SVAR_VALUE_LENGTH           128

#if !defined MAX_QUERY_LENGTH
	 #define MAX_QUERY_LENGTH           512
#endif

#define THREAD_SVARS  (15000)

#define SVARS_PREFIX                    "SVARS_"
#define LOG_PREFIX                      "SVARS"
#define SVARS_TABLE_NAME                      "svars_data"

new DB:svar_sqlite_connection = DB:false;
new svar_mysql_connection = -1;

/*
Example:

OnFilterscriptInit >
    new DB:sqlite = DB:false;
	new mysql = mysql_connect("localhost" , "samp", "database", "secretPassword");
	InitSVars(mysql, sqlite);
	for(new playerid = 0, j = GetPlayerPoolSize(); playerid <= j; playerid++)
    {
        if(IsPlayerConnected(playerid) && IsPlayerLoggedIn(playerid)) {
			LoadSVars(playerid);
		}
    }

OnPlayerDisconnect[] >
	SaveSVars(playerid);
	
OnPlayerLogin(playerid) >
	LoadSVars(plyaerid);
	
OnFilterScriptExit >
	for(new playerid = 0, j = GetPlayerPoolSize(); playerid <= j; playerid++)
    {
        if(IsPlayerConnected(playerid)) {
			SaveSVars(playerid);
		}
    }
    ExitSVars();
    

*/

/*
native SetSVarInt(playerid, varname[], int_value);
native GetSVarInt(playerid, varname[]);
native SetSVarString(playerid, varname[], string_value[]);
native GetSVarString(playerid, varname[], string_return[], len);
native SetSVarFloat(playerid, varname[], Float:float_value);
native Float:GetSVarFloat(playerid, varname[]);
native DeleteSVar(playerid, varname[]);
native SaveSVars(playerid);
native LoadSVars(playerid);
native InitSVars(mysqlConnectionHandle, DB:sqlite);
native ExitSVars();
*/

stock SVarsConnectionError() {
    printf("%s - Include did not get any connection! Please read more about InitSVars(...);\n", LOG_PREFIX);
    return false;
}

stock GenerateSVarName(varname[]){
	new svarname[MAX_SVAR_LENGTH];
	format(svarname, MAX_SVAR_LENGTH, "%s%s", SVARS_PREFIX, varname);
	return svarname;
}

stock getSVarName(svarname[]){
	new varname[MAX_SVAR_LENGTH];
	strmid(varname, svarname, strlen(SVARS_PREFIX), strlen(svarname), sizeof(varname));
	return varname;
}


forward SetSVarInt(playerid, varname[], int_value);
public SetSVarInt(playerid, varname[], int_value) {
	new svarname[MAX_SVAR_LENGTH];
	format(svarname, MAX_SVAR_LENGTH, "%s", GenerateSVarName(varname));
	return SetPVarInt(playerid, svarname, int_value);
}

forward GetSVarInt(playerid, varname[]);
public GetSVarInt(playerid, varname[]) {
    new svarname[MAX_SVAR_LENGTH];
	format(svarname, MAX_SVAR_LENGTH, "%s", GenerateSVarName(varname));
	return GetPVarInt(playerid, svarname);
}

forward SetSVarString(playerid, varname[], string_value[]);
public SetSVarString(playerid, varname[], string_value[]) {
    new svarname[MAX_SVAR_LENGTH];
	format(svarname, MAX_SVAR_LENGTH, "%s", GenerateSVarName(varname));
	return SetPVarString(playerid, svarname, string_value);
}

forward GetSVarString(playerid, varname[], string_return[], len);
public GetSVarString(playerid, varname[], string_return[], len) {
    new svarname[MAX_SVAR_LENGTH];
	format(svarname, MAX_SVAR_LENGTH, "%s", GenerateSVarName(varname));
	return GetPVarString(playerid, svarname, string_return, len);
}

forward SetSVarFloat(playerid, varname[], Float:float_value);
public SetSVarFloat(playerid, varname[], Float:float_value) {
    new svarname[MAX_SVAR_LENGTH];
	format(svarname, MAX_SVAR_LENGTH, "%s", GenerateSVarName(varname));
	return SetPVarFloat(playerid, svarname, float_value);
}

forward Float:GetSVarFloat(playerid, varname[]);
public Float:GetSVarFloat(playerid, varname[]) {
    new svarname[MAX_SVAR_LENGTH];
	format(svarname, MAX_SVAR_LENGTH, "%s", GenerateSVarName(varname));
	return GetPVarFloat(playerid, svarname);
}

forward DeleteSVar(playerid, varname[]);
public DeleteSVar(playerid, varname[]) {
    new svarname[MAX_SVAR_LENGTH];
	format(svarname, MAX_SVAR_LENGTH, "%s", GenerateSVarName(varname));
	return DeletePVar(playerid, svarname);
}

forward SaveSVars(playerid);
public SaveSVars(playerid) {
	new pVarUpperIndex = GetPVarsUpperIndex(playerid)+1;
	new sVarCount;
	new query[MAX_QUERY_LENGTH];
	new playerName[MAX_PLAYER_NAME];
    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);
    
    new pVarStringValue[MAX_SVAR_VALUE_LENGTH];
    new pVarName[MAX_SVAR_LENGTH];
    
	for(new i=0; i != pVarUpperIndex; i++)
	{
	    
	    GetPVarNameAtIndex(playerid, i, pVarName, sizeof(pVarName));
	    if(strcmp(pVarName, SVARS_PREFIX, false, strlen(SVARS_PREFIX)) != 0) continue;
	    if(GetPVarType(playerid, pVarName) != 0) sVarCount++;
		new pVarType = GetPVarType(playerid, pVarName);
		switch(pVarType)
    	{
	        case PLAYER_VARTYPE_NONE:
	        {
	            printf("%s - SVariable %s doesnt exist anymore!", LOG_PREFIX, pVarName);
	            continue;
	        }
	        case PLAYER_VARTYPE_INT:
	        {
                format(pVarStringValue, sizeof(pVarStringValue), "%d", GetPVarInt(playerid, pVarName));
	        }
	        case PLAYER_VARTYPE_FLOAT:
	        {
	            format(pVarStringValue, sizeof(pVarStringValue), "%f", GetPVarFloat(playerid, pVarName));
	        }
	        case PLAYER_VARTYPE_STRING:
	        {
                GetPVarString(playerid, pVarName, pVarStringValue, sizeof(pVarStringValue));
	        }
        }
        format(query, MAX_QUERY_LENGTH, "DELETE FROM %s WHERE key='%s' AND playername='%s';INSERT INTO %s (playername, type, key, value) VALUES (%s, %d, '%s', '%s');", \
                                                   SVARS_TABLE_NAME,   pVarName,  		playerName,   SVARS_TABLE_NAME,                 playerName, pVarType, getSVarName(pVarName), pVarStringValue);
		if(svar_mysql_connection) {
		    mysql_query(query, THREAD_SVARS, playerid, svar_mysql_connection);
		}
		else if(svar_sqlite_connection) {
		    db_query(svar_sqlite_connection, query);
		}
		else return SVarsConnectionError();
	}
	printf("%s - Saving %d Svars complete!", LOG_PREFIX, sVarCount);
	return true;
}

forward LoadSVars(playerid);
public LoadSVars(playerid) {
    new playerName[MAX_PLAYER_NAME];
	new query[MAX_QUERY_LENGTH];
	new helpStr[MAX_SVAR_LENGTH];
	
    GetPlayerName(playerid, playerName, MAX_PLAYER_NAME);
    format(query, MAX_QUERY_LENGTH, "SELECT * FROM %s WHERE playername='%s'", SVARS_TABLE_NAME, playerName);
    
    new type, key[MAX_SVAR_LENGTH], value[MAX_SVAR_VALUE_LENGTH];
    
    if(svar_mysql_connection) {
	    mysql_query(query, THREAD_SVARS, playerid, svar_mysql_connection);
	    /* @TODO: mysql fetch a nahrání jako u sqlite */
	    mysql_store_result(svar_mysql_connection);
		for(new a = 0; a<mysql_num_rows(svar_mysql_connection); a++){
			mysql_retrieve_row(svar_mysql_connection);
			mysql_fetch_field_row(helpStr, "type", svar_mysql_connection);type=strval(helpStr);
			mysql_fetch_field_row(key, "key", svar_mysql_connection);
			mysql_fetch_field_row(value, "value", svar_mysql_connection);
	    	switch(type)
			{
			    case PLAYER_VARTYPE_INT:{
					SetSVarInt(playerid, key, strval(value));
				}
			    case PLAYER_VARTYPE_FLOAT:{
	                SetSVarFloat(playerid, key, floatstr(value));
				}
				case PLAYER_VARTYPE_STRING:{
					SetSVarString(playerid, key, value);
				}
			    default: {}
			}
		}
		mysql_free_result(svar_mysql_connection);
	}
	else if(svar_sqlite_connection) {
		new DBResult: result = db_query(svar_sqlite_connection, query);
		for(new a = 0; a<db_num_rows(result); a++){
		    db_get_field_assoc(result, "type", helpStr, sizeof(helpStr)); type=strval(helpStr);
		    db_get_field_assoc(result, "key", key, sizeof(key));
		    db_get_field_assoc(result, "value", value, sizeof(value));

		    switch(type)
    		{
    		    case PLAYER_VARTYPE_INT:{
					SetSVarInt(playerid, key, strval(value));
				}
    		    case PLAYER_VARTYPE_FLOAT:{
                    SetSVarFloat(playerid, key, floatstr(value));
				}
				case PLAYER_VARTYPE_STRING:{
					SetSVarString(playerid, key, value);
				}
    		    default: {}
    		}
		    
			db_next_row(result);
		}
		db_free_result(result);
	}
	else return SVarsConnectionError();
    return true;
}

forward InitSVars(mysqlConnectionHandle, DB:sqlite);
public InitSVars(mysqlConnectionHandle, DB:sqlite) {
	if(mysqlConnectionHandle) {
	    svar_mysql_connection = mysqlConnectionHandle;
        svar_sqlite_connection = DB:false;
	}
	else if(sqlite){
	    svar_mysql_connection = -1;
        svar_sqlite_connection = sqlite;
	} else {
		return SVarsConnectionError();
	}
	printf("%s - Connection opened! Using Database and PVar Prefix [%s]\n", LOG_PREFIX, SVARS_PREFIX);
	return true;
}

forward ExitSVars();
public ExitSVars() {
    if(svar_mysql_connection) {
		svar_mysql_connection = -1;
		printf("%s - Mysql connection closed!\n", LOG_PREFIX);
	}
    else if(svar_sqlite_connection) {
		svar_sqlite_connection = DB:false;
		printf("%s - Sqlite connection closed!\n", LOG_PREFIX);
	}
	else return false;
	return true;
}

