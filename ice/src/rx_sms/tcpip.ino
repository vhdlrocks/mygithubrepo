


void initTCPIP(void)
{
  // setup TCP/IP connection
  
  // get IP address
  Serial1.println("AT#SGACT=1,1");
  delay(250);
  while(PrintModemResponse() > 0);
  while(!serial1WaitFor("OK"));
        
  //configure Exosite HTTP POST server configuration
  Serial1.println("AT#HTTPCFG=1,\"m2.exosite.com\",80,0");
//  Serial1.println("AT#HTTPCFG=1,\"m2.exosite.com/onep:v1/stack/alias?\",80,0");
  
  while(!serial1WaitFor("OK"));
}




