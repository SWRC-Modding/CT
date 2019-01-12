class HelloWeb extends WebApplication;

/* Usage:
This is a sample web application, to demonstrate how to program for the web server.


[UWeb.WebServer]
Applications[0]="UWeb.HelloWeb"
ApplicationPaths[0]="/hello"
bEnabled=True

http://server.ip.address/hello

*/

event Query(WebRequest Request, WebResponse Response)
{
	local int i;

	if(Request.Username != "test" || Request.Password != "test")
	{
		Response.FailAuthentication("HelloWeb");
		return;
	}		

	switch(Request.URI)
	{
	case "/form.html":
		Response.SendText("<form method=post action=submit.html>");
		Response.SendText("<input type=edit name=TestEdit>");
		Response.SendText("<p><select multiple name=selecter>");
		Response.SendText("<option value=\"one\">Number One");
		Response.SendText("<option value=\"two\">Number Two");
		Response.SendText("<option value=\"three\">Number Three");
		Response.SendText("<option value=\"four\">Number Four");
		Response.SendText("</select><p>");
		Response.SendText("<input type=submit name=Submit value=Submit>");
		Response.SendText("</form>");
		break;
	case "/submit.html":
		Response.SendText("Thanks for submitting the form.<br>");
		Response.SendText("TestEdit was \""$Request.GetVariable("TestEdit")$"\"<p>");
		Response.SendText("You selected these items:<br>");
		for(i=Request.GetVariableCount("selecter")-1;i>=0;i--)
			Response.SendText("\""$Request.GetVariableNumber("selecter", i)$"\"<br>");
		break;
	case "/include.html":
		Response.Subst("variable1", "This is variable 1");
		Response.Subst("variable2", "This is variable 2");
		Response.Subst("variable3", "This is variable 3");
		Response.IncludeUHTM("testinclude.html");
		break;
	default:		
		Response.SendText("Hello web!  The current level is "$Level.Title);
		Response.SendText("<br>Click <a href=\"form.html\">this link</a> to go to a test form");
		break;
	}
}

