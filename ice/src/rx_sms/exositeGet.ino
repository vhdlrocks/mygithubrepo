


void exositeGet(void)
{
  Serial1.println(F("AT#SGACT = 1,1"));
  while(!serial1WaitFor("OK")); 
  
  Serial1.println(F("AT#SD=3,0,80,\"m2.exosite.com\""));
  delay(5000); 
  
  Serial1.print(F("GET /onep:v1/stack/alias?OnOffSw HTTP/1.1\n"));
  delay(500); 
  Serial1.print(F("Accept: application/x-www-form-urlencoded; charset=utf-8\n"));
  delay(500); 
  Serial1.print(F("Accept-Encoding: gzip, deflate\n"));
  delay(500); 
  Serial1.print(F("Host: m2.exosite.com\n"));
  delay(500); 
  Serial1.print(F("Connection: Close\n"));
  delay(500); 
  Serial1.print(F("X-Exosite-CIK: 65930c137a0eefc5433357447f9fbbce33a80795\n\n\n"));
  while(!serial1WaitFor("NO CARRIER"));
//  while(PrintModemResponse() > 0);
  delay(5000);
  
  
  
  
  Serial1.println(F("AT#SD=3,0,80,\"m2.exosite.com\""));
  delay(5000); 
  
  Serial1.print(F("POST /onep:v1/stack/alias HTTP/1.1\n"));
  delay(500); 
  Serial1.print(F("Host: m2.exosite.com\n"));
  delay(500); 
  Serial1.print(F("X-Exosite-CIK: 7cafee22ab8628b2838187a7774f5e4b3f05f877\n"));
  delay(500); 
  Serial1.print(F("Content-Type: application/x-www-form-urlencoded; charset=utf-8\n"));
  delay(500); 
  Serial1.print(F("Content-Length: 28\n\n"));
  delay(500); 
  Serial1.print(F("Key1=abcd&Key2=123&Key3=45.6"));
  while(1)PrintModemResponse();
  
  

  
}




