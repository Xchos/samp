/*==============================================================================


	Authorizator 1.0.1

		A SA:MP library for saving player progress into database for statistics.

		Library originally written by Lukas Zelenka: lukas.zelenka974@gmail.com


==============================================================================*/

#if defined _auth_included
    #endinput
#endif
#define _auth_included

#if !defined _samp_included
	#include <a_samp>
#endif

#define MAX_AUTH_QUERY_LENGTH 350
#define MAX_AUTH_NAME_LENGTH 100

new DB:AUTHDB;
new AUTHVERBOSE = 1;
/* NATIVES
	native authQueryLog(state); // 1 prints to log ; 0 disable printing queries
	native authInit(name[]);
	native authExit();

	native IsPlayerUniqueAuthsIDAssigned(playerid); // returns true/false
	native AssignPlayerUniqueAuthID(playerid); // returns (int)uniqueID
	native GetPlayerUniqueAuthID(playerid); // returns (int)uniqueID
	
	native CreatePlayerAccount(playerid, password[]);
	native PlayerAccountExists(playerid);
	native IsPlayerAuthorized(playerid);
	native AuthenticatePlayerAccount(playerid, password[]);
	native UnauthenticatePlayerAccount(playerid);
*/



stock authBeginTransaction(){
    if(AUTHVERBOSE) printf("[AUTHDB transaction] started");
    db_query(AUTHDB, "BEGIN TRANSACTION");
    return 1;
}

stock authCommit(){
    if(AUTHVERBOSE) printf("[AUTHDB transaction] commited");
    db_query(AUTHDB, "COMMIT");
    return 1;
}

stock authRollback(){
    if(AUTHVERBOSE) printf("[AUTHDB transaction] canceled");
    db_query(AUTHDB, "ROLLBACK");
    return 1;
}
stock DBResult:authQuery(query[]){
	if(AUTHVERBOSE) printf("[AUTHDB query] %s", query);
	return db_query(AUTHDB, query);
}


stock authEscape(text[])
{
	new
		ret[MAX_AUTH_QUERY_LENGTH * 2],
		ch,
		i,
		j;
	while ((ch = text[i++]) && j < sizeof (ret))
	{
		if (ch == '\'')
		{
			if (j < sizeof (ret) - 2)
			{
				ret[j++] = '\'';
				ret[j++] = '\'';
			}
		}
		else if (j < sizeof (ret))
		{
			ret[j++] = ch;
		}
		else
		{
			j++;
		}
	}
	ret[sizeof (ret) - 1] = '\0';
	return ret;
}

// functions

forward authQueryLog(verbose);
public authQueryLog(verbose){
    AUTHVERBOSE=verbose;
    return 1;
}

forward authInit(name[]);
public authInit(name[]){
	AUTHDB = db_open(name);
    authBeginTransaction();
    authQuery("CREATE TABLE IF NOT EXISTS auth_accounts(uniqueid INTEGER PRIMARY KEY NOT NULL, password TEXT NOT NULL);");
    authQuery("CREATE TABLE IF NOT EXISTS uid_generator(id INTEGER PRIMARY KEY NOT NULL, name TEXT);"); // auth Compatibility
    authCommit();
	return 1;
}

forward authExit();
public authExit(){
	db_close(AUTHDB);
}

forward IsPlayerUniqueAuthIDAssigned(playerid);
public IsPlayerUniqueAuthIDAssigned(playerid){
    if(!GetPVarType(playerid, "auth_uniqueID")) return false;
    else return true;
}

forward AssignPlayerUniqueAuthID(playerid); // Will generate new UID when UID is not generated yet.
public AssignPlayerUniqueAuthID(playerid){
	new query[MAX_AUTH_QUERY_LENGTH];
	new name[MAX_PLAYER_NAME];
	new HELP_STR[30];
	GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	format(query, MAX_AUTH_QUERY_LENGTH, "SELECT id FROM uid_generator WHERE name LIKE '%s'", name);
	new DBResult:result = authQuery(query);
	if(!db_num_rows(result)){
		format(query, MAX_AUTH_QUERY_LENGTH, "INSERT INTO uid_generator (name) VALUES ('%s');", name);
		authQuery(query);

		db_free_result(result);

		format(query, MAX_AUTH_QUERY_LENGTH, "SELECT id FROM uid_generator WHERE name LIKE '%s'", name);
		result = authQuery(query);
	}

	db_get_field(result, 0, HELP_STR, 30);
	db_free_result(result);
	new uniqueid = strval(HELP_STR);
	SetPVarInt(playerid, "auth_uniqueID", uniqueid);
	return uniqueid;
}

forward GetPlayerUniqueAuthID(playerid);
public GetPlayerUniqueAuthID(playerid){
	return GetPVarInt(playerid, "auth_uniqueID");
}

forward IsPlayerAuthorized(playerid);
public IsPlayerAuthorized(playerid){
	return GetPVarInt(playerid, "auth_authorized");
}

forward PlayerAccountExists(playerid);
public PlayerAccountExists(playerid){
    if(!IsPlayerUniqueAuthIDAssigned(playerid)) return false;
	new uniqueid = GetPlayerUniqueAuthID(playerid);
	new query[MAX_AUTH_QUERY_LENGTH];
	format(query, MAX_AUTH_QUERY_LENGTH, "SELECT * FROM auth_accounts WHERE uniqueid=%d LIMIT 1;", uniqueid);
	new DBResult: result = authQuery(query);
	if(db_num_rows(result)) return true;
	else return false;
}

forward CreatePlayerAccount(playerid, password[]);
public CreatePlayerAccount(playerid, password[]){
	if(!IsPlayerUniqueAuthIDAssigned(playerid)) return false;
	new uniqueid = GetPlayerUniqueAuthID(playerid);
	new query[MAX_AUTH_QUERY_LENGTH];
	new eString[MAX_AUTH_QUERY_LENGTH];
	format(eString, MAX_AUTH_QUERY_LENGTH, "%s", authEscape(password));
	
	format(query, MAX_AUTH_QUERY_LENGTH, "INSERT OR REPLACE INTO auth_accounts (uniqueid, password) VALUES (%d, '%s');", uniqueid, eString);
	authQuery(query);
	return true;
}

forward AuthenticatePlayerAccount(playerid, password[]);
public AuthenticatePlayerAccount(playerid, password[]){
	if(!IsPlayerUniqueAuthIDAssigned(playerid)) return false;
	new uniqueid = GetPlayerUniqueAuthID(playerid);
	new query[MAX_AUTH_QUERY_LENGTH];
	new eString[MAX_AUTH_QUERY_LENGTH];
	format(eString, MAX_AUTH_QUERY_LENGTH, "%s", authEscape(password));

	format(query, MAX_AUTH_QUERY_LENGTH, "SELECT * FROM auth_accounts WHERE uniqueid=%d AND password LIKE '%s' LIMIT 1;", uniqueid, eString);
	new DBResult: result = authQuery(query);
	if(db_num_rows(result)) SetPVarInt(playerid, "auth_authorized", 1);
	else DeletePVar(playerid, "auth_authorized");
	return IsPlayerAuthorized(playerid);
}

forward UnauthenticatePlayerAccount(playerid);
public UnauthenticatePlayerAccount(playerid){
    DeletePVar(playerid, "auth_authorized");
	return true;
}

