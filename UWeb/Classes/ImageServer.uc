class ImageServer extends WebApplication;

/* Usage:
[UWeb.WebServer]
Applications[0]="UWeb.ImageServer"
ApplicationPaths[0]="/images"
bEnabled=True

http://server.ip.address/images/test.jpg
*/

event Query(WebRequest Request, WebResponse Response)
{
	local string Image;
	
	Image = Mid(Request.URI, 1);
	if( Right(Caps(Image), 4) == ".JPG" || Right(Caps(Image), 5) == ".JPEG" )
	{
		Response.SendStandardHeaders("image/jpeg", true);
	}
	else if( Right(Caps(Image), 4) == ".GIF" )
	{
		Response.SendStandardHeaders("image/gif", true);
	}
	else if( Right(Caps(Image), 4) == ".BMP" )
	{
		Response.SendStandardHeaders("image/bmp", true);
	}
	else
	{
		Response.HTTPError(404);
		return;
	}
	Response.IncludeBinaryFile( "images/"$Image );
}

