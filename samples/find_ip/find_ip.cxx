//
// find_ip.cxx
//
// When used behind NAT, this finds the IP address of the NAT box.
//
// Roger Hardiman
//

#include <ptlib.h>
#include <ptclib/http.h>
#include <ptclib/html.h>

class FindIP : public PProcess
{
  PCLASSINFO(FindIP, PProcess)
  public:
    void Main();
    BOOL get_ip(PString server_url, PString &ip_address);
};

PCREATE_PROCESS(FindIP)

void FindIP::Main()
{
  cout << "Find IP - Find the IP address of your NAT box" << endl;
  PString ip;

  if (get_ip("http://checkip.dyndns.org:8245/",ip)) {
    cout << ip << endl;
  }

  if (get_ip("http://checkip.dyndns.org/",ip)) {
    cout << ip << endl;
  }

  if (get_ip("http://www.ip-calculator.com/",ip)) {
    cout << ip << endl;
  }

  if (get_ip("http://www.lawrencegoetz.com/programs/ipinfo/",ip)) {
    cout << ip << endl;
  }

  if (get_ip("http://www.whatismyipaddress.com/",ip)) {
    cout << ip << endl;
  }
}

BOOL FindIP::get_ip(PString server_url, PString &ip_address){

  cout << "----------------------------------------" << endl;
  cout << "Connecting to " << server_url << endl;

  BOOL result = FALSE;

  PHTTPClient web("webserver");
  PINDEX len;

  if (web.GetDocument(server_url,len)) {
    PString html = web.ReadString(len);
    if (!html.IsEmpty()) {
      // cout << "The web server returned the data" << endl << html << endl;
      // Now parse the HTML and pull out the IP address. Just look
      // for anything of the form N.N.N.N where N is an integer.
      PRegularExpression regex("[0-9]*[.][0-9]*[.][0-9]*[.][0-9]*");
      PINDEX pos,len;
      if (html.FindRegEx(regex, pos, len)) {
        ip_address = html.Mid(pos,len);
        cout << "Your real IP is " << ip_address << endl;
	result = TRUE;
      }
    } else {
      cout << "Page is empty." << endl;
    }
  } else {
    cout << "Could not get page from server." << endl;
  }
  cout << "----------------------------------------" << endl;
  return result;
}

// End of find_ip.cxx
