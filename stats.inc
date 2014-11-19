/*==============================================================================


	Statistics 1.0.1

		A SA:MP library for saving player progress into database for statistics.

		Library originally written by Lukas Zelenka: lukaszelenka.974@gmail.com


==============================================================================*/

#if defined _stats_included
    #endinput
#endif
#define _stats_included

#if !defined _samp_included
	#include <a_samp>
#endif

#define MAX_STATS_QUERY_LENGTH 350
#define MAX_STATS_NAME_LENGTH 100

new DB:STATSDB;
new STATSVERBOSE = 1;
/* NATIVES
	native queryLog(bool:state);
	native statsInit(name[]);
	native statsExit();

	native GetStatName(statid); // returns string[]
	native SetStatName(statid, name[]); // returns true/false

	native IsPlayerUniqueStatsIDAssigned(playerid); // returns true/false
	native AssignPlayerUniqueStatsID(playerid); // returns (int)uniqueID
	native GetPlayerUniqueStatsID(playerid); // returns (int)uniqueID

	native SetPlayerIntegerStat(playerid, statid, value); //returns true/false
	native GetPlayerIntegerStat(playerid, statid); //returns (int)value
	native SetPlayerFloatStat(playerid, statid, Float:value); //returns true/false
	native Float:GetPlayerFloatStat(playerid, statid); //returns (float)value
*/



stock statsBeginTransaction(){
    if(STATSVERBOSE) printf("[STATSDB transaction] started");
    db_query(STATSDB, "BEGIN TRANSACTION");
    return 1;
}

stock statsCommit(){
    if(STATSVERBOSE) printf("[STATSDB transaction] commited");
    db_query(STATSDB, "COMMIT");
    return 1;
}

stock statsRollback(){
    if(STATSVERBOSE) printf("[STATSDB transaction] canceled");
    db_query(STATSDB, "ROLLBACK");
    return 1;
}
stock DBResult:statsQuery(query[]){
	if(STATSVERBOSE) printf("[STATSDB query] %s", query);
	return db_query(STATSDB, query);
}

stock StatCacheName(statid){
    new pname[64];
	format(pname, 64, "stats_%d", statid);
	return pname;
}

stock IsPlayerIntegerStatInCache(playerid, statid){

	if(GetPVarType(playerid, StatCacheName(statid)) == PLAYER_VARTYPE_INT) return true;
    else return false;
}

stock GetPlayerIntegerStatFromCache(playerid, statid){
	return GetPVarInt(playerid, StatCacheName(statid));
}

stock IsPlayerFloatStatInCache(playerid, statid){
	if(GetPVarType(playerid, StatCacheName(statid)) == PLAYER_VARTYPE_FLOAT) return true;
    else return false;
}

stock Float:GetPlayerFloatStatFromCache(playerid, statid){
	return GetPVarFloat(playerid, StatCacheName(statid));
}

stock statsEscape(text[])
{
	new
		ret[MAX_STATS_NAME_LENGTH * 2],
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

forward queryLog(verbose);
public queryLog(verbose){
    STATSVERBOSE=verbose;
    return 1;
}

forward statsInit(name[]);
public statsInit(name[]){
	new exists = 0;
	if(fexist(name)) exists = 1;
	STATSDB = db_open(name);
	if(!exists){
	    statsBeginTransaction();
	    statsQuery("CREATE TABLE IF NOT EXISTS player_i(id INTEGER PRIMARY KEY NOT NULL, uniqueid INTEGER NOT NULL, statid INTEGER NOT NULL, value INTEGER);");
	    statsQuery("CREATE TABLE IF NOT EXISTS player_f(id INTEGER PRIMARY KEY NOT NULL, uniqueid INTEGER NOT NULL, statid INTEGER NOT NULL, value REAL);");
	    statsQuery("CREATE TABLE IF NOT EXISTS stats(id INTEGER PRIMARY KEY NOT NULL, name TEXT);");
	    statsQuery("CREATE TABLE IF NOT EXISTS uid_generator(id INTEGER PRIMARY KEY NOT NULL, name TEXT);");
	    statsCommit();
	}
	return 1;
}

forward statsExit();
public statsExit(){
	db_close(STATSDB);
}

forward SetStatName(statid, name[]);
public SetStatName(statid, name[]){
	new eString[MAX_STATS_NAME_LENGTH];
	format(eString, MAX_STATS_NAME_LENGTH, "%s", statsEscape(name));
	new query[MAX_STATS_QUERY_LENGTH];
	format(query, MAX_STATS_QUERY_LENGTH, "INSERT OR REPLACE INTO stats (id,name) VALUES (%d, '%s');", statid, eString);
	statsQuery(query);
}

stock GetStatName(statid){
	new query[MAX_STATS_QUERY_LENGTH];
	new statsName[MAX_STATS_NAME_LENGTH];
	format(query, MAX_STATS_QUERY_LENGTH, "SELECT name FROM stats WHERE id=%d", statid);
	new DBResult:result = statsQuery(query);
	if(!db_num_rows(result)) format(statsName, 128, "[STATS ERROR]");
	else db_get_field(result, 0, statsName, 128);
	return statsName;
}


forward IsPlayerUniqueStatsIDAssigned(playerid);
public IsPlayerUniqueStatsIDAssigned(playerid){
    if(!GetPVarType(playerid, "stats_uniqueID")) return false;
    else return true;
}

forward AssignPlayerUniqueStatsID(playerid); // Will generate new UID when UID is not generated yet.
public AssignPlayerUniqueStatsID(playerid){
	new query[MAX_STATS_QUERY_LENGTH];
	new name[MAX_PLAYER_NAME];
	new HELP_STR[30];
	GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	format(query, MAX_STATS_QUERY_LENGTH, "SELECT id FROM uid_generator WHERE name LIKE '%s'", name);
	new DBResult:result = statsQuery(query);
	if(!db_num_rows(result)){
		format(query, MAX_STATS_QUERY_LENGTH, "INSERT INTO uid_generator (name) VALUES ('%s');", name);
		statsQuery(query);

		db_free_result(result);

		format(query, MAX_STATS_QUERY_LENGTH, "SELECT id FROM uid_generator WHERE name LIKE '%s'", name);
		result = statsQuery(query);
	}

	db_get_field(result, 0, HELP_STR, 30);
	db_free_result(result);
	new uniqueid = strval(HELP_STR);
	SetPVarInt(playerid, "stats_uniqueID", uniqueid);
	return uniqueid;
}

forward GetPlayerUniqueStatsID(playerid);
public GetPlayerUniqueStatsID(playerid){
	return GetPVarInt(playerid, "stats_uniqueID");
}

forward SetPlayerIntegerStat(playerid, statid, value);
public SetPlayerIntegerStat(playerid, statid, value){
	if(!IsPlayerUniqueStatsIDAssigned(playerid)) return false;

	new exists=0;
	new uniqueid=GetPlayerUniqueStatsID(playerid);
	new query[MAX_STATS_QUERY_LENGTH];

	if(!IsPlayerIntegerStatInCache(playerid, statid)) {
		format(query, MAX_STATS_QUERY_LENGTH, "SELECT * FROM player_i WHERE statid=%d AND uniqueid=%d", statid, uniqueid);
		if(db_num_rows(statsQuery(query))) exists=1;
		else exists = 0;
	} else exists = 1;


	if(exists){
	    format(query, MAX_STATS_QUERY_LENGTH, "UPDATE player_i SET value=%d WHERE statid=%d AND uniqueid=%d", value, statid, uniqueid);
	} else {
	    format(query, MAX_STATS_QUERY_LENGTH, "INSERT INTO player_i (uniqueid,statid,value) VALUES ('%d','%d','%d');",uniqueid,statid,value);
	}
	statsQuery(query);
	SetPVarInt(playerid, StatCacheName(statid), value);
	return true;
}


forward GetPlayerIntegerStat(playerid, statid);
public GetPlayerIntegerStat(playerid, statid){
	if(!IsPlayerUniqueStatsIDAssigned(playerid)) return 0;

	new uniqueid=GetPlayerUniqueStatsID(playerid);
	new query[MAX_STATS_QUERY_LENGTH];
	new value;

	if(IsPlayerIntegerStatInCache(playerid, statid)) value = GetPlayerIntegerStatFromCache(playerid, statid);
	else {
	    format(query, MAX_STATS_QUERY_LENGTH, "SELECT value FROM player_i WHERE uniqueid=%d AND statid=%d",uniqueid,statid);
		new HELP_STR[30];
		new DBResult:result = statsQuery(query);
		if(!db_num_rows(result)) return 0;
		db_get_field(result, 0, HELP_STR, 30);
		value = strval(HELP_STR);
		SetPVarInt(playerid, StatCacheName(statid), value);
	}
	return value;
}



forward SetPlayerFloatStat(playerid, statid, Float:value);
public SetPlayerFloatStat(playerid, statid, Float:value){
	if(!IsPlayerUniqueStatsIDAssigned(playerid)) return 0;

	new exists=0;
	new uniqueid=GetPlayerUniqueStatsID(playerid);
	new query[MAX_STATS_QUERY_LENGTH];

	if(!IsPlayerFloatStatInCache(playerid, statid)) {
		format(query, MAX_STATS_QUERY_LENGTH, "SELECT * FROM player_f WHERE statid=%d AND uniqueid=%d", statid, uniqueid);
		if(db_num_rows(statsQuery(query))) exists=1;
		else exists = 0;
	} else exists = 1;


	if(exists){
	    format(query, MAX_STATS_QUERY_LENGTH, "UPDATE player_f SET value=%f WHERE statid=%d AND uniqueid=%d", value, statid, uniqueid);
	} else {
	    format(query, MAX_STATS_QUERY_LENGTH, "INSERT INTO player_f (uniqueid,statid,value) VALUES ('%d','%d','%f');",uniqueid,statid,value);
	}
	statsQuery(query);
	SetPVarFloat(playerid, StatCacheName(statid), value);
	return true;
}

forward Float:GetPlayerFloatStat(playerid, statid);
public Float:GetPlayerFloatStat(playerid, statid){
	if(!IsPlayerUniqueStatsIDAssigned(playerid)) return 0.0;

	new uniqueid=GetPlayerUniqueStatsID(playerid);
	new query[MAX_STATS_QUERY_LENGTH];
	new Float:value;

	if(IsPlayerFloatStatInCache(playerid, statid)) value = GetPlayerFloatStatFromCache(playerid, statid);
	else {
	    format(query, MAX_STATS_QUERY_LENGTH, "SELECT value FROM player_f WHERE uniqueid=%d AND statid=%d",uniqueid,statid);
		new HELP_STR[30];
		new DBResult:result = statsQuery(query);
		if(!db_num_rows(result)) return 0.0;
		db_get_field(result, 0, HELP_STR, 30);
		value = floatstr(HELP_STR);
		SetPVarFloat(playerid, StatCacheName(statid), value);
	}
	return value;
}
