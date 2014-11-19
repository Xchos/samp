Úvod:
-----
Napsal jsem jednoduchou knihovnu pro zavedení statistik do hry. Je to psané pro širokou škálu lidí, takže jsem v tvorbě koukal na univerzálnost. Pro využití je zapotřebí pouze jednoho souboru ve složce scriptfiles, který si include sám vytvoří, jakmile bude použit. Hráči je přiřazováno unikátní ID, které slouží pro zavedení statistik. Toto ID je generováno na herní přezdívku. Nebudu protahovat a tady uvedu jednoduché příklady implementace:

Implementace:
-------------

do OnGameModeInit() případně OnFilterScriptInit() uvedeme následující:
~~
	statsInit("stats.sqlite");

	statsBeginTransaction();        // Začne transakci - vykoná všechny query najednou. V tomto případě registraci všech názvů statistik.

	SetStatName(1, "Připojení");
	SetStatName(2, "Nastoupení do vozu");
	SetStatName(3, "nejvyšší rychlost");
	
	statsCommit();
~~
Jedná se o inicializaci, takže by se měla dodržet konvence o "very first", tedy jako první pokud možno.

Poté je potřeba ošetřit zavření databáze. To učiníme v OnGameModeExit() případně OnFilterScriptExit() ..
~~
statsExit();
~~

Pak už se dá jen jednoduše pracovat. Include využívá hojně PlayerVariables a to s prefixem stats_. Do těchto proměnných si ukládá Cache. Cache slouží k tomu, aby se redukovali co nejvíce režijní náklady na databázi.

Soubor funkcí:
--------------
Funkce, které include obsahuje:
~~
	native queryLog(bool:state); // slouží k printu dotazů na databázi do server_log.log
	native statsInit(name[]); // inicializace statistik
	native statsExit(); // odhlášení od statistik
	
	native GetStatName(statid); // returns string[]
	native SetStatName(statid, name[]); // returns true/false

	native IsPlayerUniqueStatsIDAssigned(playerid); // returns true/false
	native AssignPlayerUniqueStatsID(playerid); // returns (int)uniqueID
	native GetPlayerUniqueStatsID(playerid); // returns (int)uniqueID
	
	native SetPlayerIntegerStat(playerid, statid, value); //returns true/false
	native GetPlayerIntegerStat(playerid, statid); //returns (int)value
	native SetPlayerFloatStat(playerid, statid, Float:value); //returns true/false
	native Float:GetPlayerFloatStat(playerid, statid); //returns (float)value


*Poprosím všechny, co se tuto knihovnu rozhodnou využít aby mě zanechali jako autora tohoto scriptu. Děkuji!*
