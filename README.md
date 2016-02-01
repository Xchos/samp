# samp
GTA San Andreas Multiplayer scripts
```
#include <auth>

#define DIALOG_LOGIN 1
#define DIALOG_REGISTER 2

stock ShowPlayerLoginForm(playerid, retry=0){
	formatted("Please insert password for your character [%s]", PlayerName(playerid));
	if(retry) formatted("Wrong password!\n%s", fstr);
	ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "Login form", fstr, "Login", "Quit Game");
	return 1;
}

stock ShowPlayerRegisterForm(playerid){
	formatted("Please insert password to create your account [%s].\nLenght: [4 - 16]", PlayerName(playerid));
	ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_INPUT, "Register form", fstr, "Register", "Quit Game");
	return 1;
}

public OnGameModeInit()
{
	authInit("main.sqlite");
  return 1;
}
public OnGameModeExit()
{
	authExit();
  return 1;
}
public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch(dialogid){
		case DIALOG_LOGIN:{
      if(!response) return Kick(playerid);
			AuthenticatePlayerAccount(playerid, inputtext);
			if(!IsPlayerAuthorized(playerid)) ShowPlayerLoginForm(playerid, 1);
			return 1;
		}
		case DIALOG_REGISTER:{
        if(!response) return Kick(playerid);
		    if(strlen(inputtext)<4 || strlen(inputtext)>16) return ShowPlayerRegisterForm(playerid);
		    CreatePlayerAccount(playerid, inputtext);
		    return 1;
		}
	}
	return 1;
}

public OnPlayerConnect(playerid)
{

	AssignPlayerUniqueAuthID(playerid);
	
	if(PlayerAccountExists(playerid)){
	    ShowPlayerLoginForm(playerid);
	} else ShowPlayerRegisterForm(playerid);

	
	return 1;
}
```
