class WebApplication extends Object;
	
// Set by the webserver
var LevelInfo Level;
var WebServer WebServer;
var string Path;

function Init();
function Cleanup();
function bool PreQuery(WebRequest Request, WebResponse Response) { return true; }
function Query(WebRequest Request, WebResponse Response);
function PostQuery(WebRequest Request, WebResponse Response);
