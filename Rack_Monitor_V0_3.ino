/**
 * Agentuino SNMP Agent Library Prototyping...
 *
 * Copyright 2010 Eric C. Gionet <lavco_eg@hotmail.com>
 * Modified for rack monitor project by Shaun Houghton
 */
#include <Streaming.h>         // Include the Streaming library
#include <Ethernet.h>          // Include the Ethernet library
#include <SPI.h>
#include <MemoryFree.h>
#include <Agentuino.h> 
#include <Flash.h>
//
#define DEBUG
//
static byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
static byte ip[] = { 
  192, 168, 1, 223 };
static byte gateway[] = { 
  192, 168, 1, 1 };
static byte subnet[] = { 
  255, 255, 255, 0 };

//Setup standard OID values
//
// tkmib - linux mib browser
//
// RFC1213-MIB OIDs
// .iso (.1)
// .iso.org (.1.3)
// .iso.org.dod (.1.3.6)
// .iso.org.dod.internet (.1.3.6.1)
// .iso.org.dod.internet.mgmt (.1.3.6.1.2)
// .iso.org.dod.internet.mgmt.mib-2 (.1.3.6.1.2.1)
// .iso.org.dod.internet.mgmt.mib-2.system (.1.3.6.1.2.1.1)
// .iso.org.dod.internet.mgmt.mib-2.system.sysDescr (.1.3.6.1.2.1.1.1)
static char sysDescr[] PROGMEM      = "1.3.6.1.2.1.1.1.0";  // read-only  (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysObjectID (.1.3.6.1.2.1.1.2)
static char sysObjectID[] PROGMEM   = "1.3.6.1.2.1.1.2.0";  // read-only  (ObjectIdentifier)
// .iso.org.dod.internet.mgmt.mib-2.system.sysUpTime (.1.3.6.1.2.1.1.3)
static char sysUpTime[] PROGMEM     = "1.3.6.1.2.1.1.3.0";  // read-only  (TimeTicks)
// .iso.org.dod.internet.mgmt.mib-2.system.sysContact (.1.3.6.1.2.1.1.4)
static char sysContact[] PROGMEM    = "1.3.6.1.2.1.1.4.0";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysName (.1.3.6.1.2.1.1.5)
static char sysName[] PROGMEM       = "1.3.6.1.2.1.1.5.0";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysLocation (.1.3.6.1.2.1.1.6)
static char sysLocation[] PROGMEM   = "1.3.6.1.2.1.1.6.0";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysServices (.1.3.6.1.2.1.1.7)
static char sysServices[] PROGMEM   = "1.3.6.1.2.1.1.7.0";  // read-only  (Integer)

//
// Arduino defined OIDs
// .iso.org.dod.internet.private (.1.3.6.1.4)
// .iso.org.dod.internet.private.enterprises (.1.3.6.1.4.1)
// .iso.org.dod.internet.private.enterprises.arduino (.1.3.6.1.4.1.36582)
//
//
// RFC1213 local values
static char locDescr[]              = "Rack Monitor - SNMP Agent.";  // read-only (static)
static char locObjectID[]           = ".1.3.6.1.4.1.36582.0";                       // read-only (static)
static uint32_t locUpTime           = 0;                                        // read-only (static)
static char locContact[20]          = "Shaun Houghton";                            // should be stored/read from EEPROM - read/write (not done for simplicity)
static char locName[20]             = "MDD Rack";                              // should be stored/read from EEPROM - read/write (not done for simplicity)
static char locLocation[20]         = "Pavilion";                        // should be stored/read from EEPROM - read/write (not done for simplicity)
static int32_t locServices          = 7;                                        // read-only (static)

// .iso.org.dod.internet.private.enterprises.arduino.value.valA0-A5 (.1.3.6.1.4.1.36582.3.1-6)
static char valA0[] PROGMEM   = "1.3.6.1.4.1.36582.3.1.0";  // read-only  (Integer)
static char valA1[] PROGMEM   = "1.3.6.1.4.1.36582.3.2.0";  // read-only  (Integer)
static char valA2[] PROGMEM   = "1.3.6.1.4.1.36582.3.3.0";  // read-only  (Integer)
static char valA3[] PROGMEM   = "1.3.6.1.4.1.36582.3.4.0";  // read-only  (Integer)
static char valA4[] PROGMEM   = "1.3.6.1.4.1.36582.3.5.0";  // read-only  (Integer)
static char valA5[] PROGMEM   = "1.3.6.1.4.1.36582.3.6.0";  // read-only  (Integer)

// .iso.org.dod.internet.private.enterprises.arduino.value.valD0-D13 (.1.3.6.1.4.1.36582.3.7-20)
static char valD0[] PROGMEM   = "1.3.6.1.4.1.36582.3.7.0";  // read-only  (Integer)
static char valD1[] PROGMEM   = "1.3.6.1.4.1.36582.3.8.0";  // read-only  (Integer)
static char valD2[] PROGMEM   = "1.3.6.1.4.1.36582.3.9.0";  // read-only  (Integer)
static char valD3[] PROGMEM   = "1.3.6.1.4.1.36582.3.10.0";  // read-only  (Integer)
static char valD4[] PROGMEM   = "1.3.6.1.4.1.36582.3.11.0";  // read-only  (Integer)
static char valD5[] PROGMEM   = "1.3.6.1.4.1.36582.3.12.0";  // read-only  (Integer)
static char valD6[] PROGMEM   = "1.3.6.1.4.1.36582.3.13.0";  // read-only  (Integer)

uint32_t prevMillis = millis();
char oid[SNMP_MAX_OID_LEN];
SNMP_API_STAT_CODES api_status;
SNMP_ERR_CODES status;

void pduReceived()
{
  SNMP_PDU pduRequest;
  SNMP_PDU pduResponse;
  
  api_status = Agentuino.requestPdu(&pduRequest);
  pduRequest.OID.toString(oid);
  
  Serial.print("API Status is ");
  Serial.println(api_status);
  Serial.print("PDU type is ");
  Serial.println(pduRequest.type);
  Serial.print("PDU error is ");
  Serial.println(pduRequest.error);
  Serial.print("PDU request ID is ");
  Serial.println(pduRequest.requestId);
//  Serial.print("OID is ");
//  Serial.println(pduRequest.OID);
  Serial.print("Formated OID is ");
  Serial.println(oid);
  Serial.print("SNMP Value ");
  //Serial.println(pduRequest.VALUE);
  
  
//  Serial.print("SNMP_PDU_GET = ");
//  Serial.println(SNMP_PDU_GET);
//  Serial.print("SNMP_PDU_GET_NEXT = ");
//  Serial.println(SNMP_PDU_GET_NEXT);
  Serial.print("SNMP Version ");
  Serial.println(pduRequest.version);
  
  if (pduRequest.type == SNMP_PDU_GET_NEXT){
    Serial.println("Get Next command");
    Agentuino.freePdu(&pduRequest);
    pduRequest.type = SNMP_PDU_RESPONSE;
    pduRequest.error = SNMP_ERR_READ_ONLY;
    Agentuino.responsePdu(&pduRequest);
    
    Agentuino.responsePdu(&pduRequest);
    
    return;
  }
  Serial.println("pdReived start");

  
  //
  #ifdef DEBUG
  Serial << F("UDP Packet Received Start..") << F(" RAM:") << freeMemory() << endl;
  #endif
  
  

  if ( pduRequest.type == SNMP_PDU_GET || pduRequest.type == SNMP_PDU_SET
    && pduRequest.error == SNMP_ERR_NO_ERROR && api_status == SNMP_API_STAT_SUCCESS ) {
    //
    pduRequest.OID.toString(oid);

    //
    //Serial << "OID: " << oid << endl;
    //

    if ( strcmp_P(oid, sysDescr ) == 0 ) {
      // handle sysDescr (set/get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = SNMP_ERR_READ_ONLY;
      } 
      else {
        // response packet from get-request - locDescr
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_OCTETS, locDescr);
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("sysDescr...") << locDescr << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    } 
    else if ( strcmp_P(oid, sysUpTime ) == 0 ) {
      // handle sysName (set/get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = SNMP_ERR_READ_ONLY;
      } 
      else {
        // response packet from get-request - locUpTime
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_TIME_TICKS, locUpTime);
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("sysUpTime...") << locUpTime << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    } 
    else if ( strcmp_P(oid, sysName ) == 0 ) {
      // handle sysName (set/get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pduRequest.VALUE.decode(locName, strlen(locName)); 
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      } 
      else {
        // response packet from get-request - locName
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_OCTETS, locName);
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("sysName...") << locName << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    } 
    else if ( strcmp_P(oid, sysContact ) == 0 ) {
      // handle sysContact (set/get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pduRequest.VALUE.decode(locContact, strlen(locContact)); 
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      } 
      else {
        // response packet from get-request - locContact
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_OCTETS, locContact);
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("sysContact...") << locContact << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    } 
    else if ( strcmp_P(oid, sysLocation ) == 0 ) {
      // handle sysLocation (set/get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pduRequest.VALUE.decode(locLocation, strlen(locLocation)); 
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      } 
      else {
        // response packet from get-request - locLocation
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_OCTETS, locLocation);
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("sysLocation...") << locLocation << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    } 
    else if ( strcmp_P(oid, sysServices) == 0 ) {
      // handle sysServices (set/get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = SNMP_ERR_READ_ONLY;
      } 
      else {
        // response packet from get-request - locServices
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_INT, locServices);
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("locServices...") << locServices << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    }
    else if ( strcmp_P(oid, sysObjectID) == 0 ) {
      // handle sysObjectID (set/get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = SNMP_ERR_READ_ONLY;
      } 
      else {
        // response packet from get-request - locServices
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_OID, locObjectID);
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("locObjectIF...") << locObjectID << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    } 
    else if ( strcmp_P(oid, valA0) == 0 ) {
      // handle valA0 (get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = SNMP_ERR_READ_ONLY;
      } 
      else {
        // response packet from get-request - valA0
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_INT32, (long(analogRead(0)))*500/1024);
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("valA0...") << locObjectID << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    }

    else if ( strcmp_P(oid, valD2) == 0 ) {
      // handle valD2 (get) requests
      if ( pduRequest.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = SNMP_ERR_READ_ONLY;
      } 
      else {
        // response packet from get-request - valA0
        status = pduRequest.VALUE.encode(SNMP_SYNTAX_INT32, digitalRead(2));
        pduRequest.type = SNMP_PDU_RESPONSE;
        pduRequest.error = status;
      }
      //
      #ifdef DEBUG
      Serial << F("valD2...") << locObjectID << F(" ") << pduRequest.VALUE.size << endl;
      #endif
    }

    //  End of processing code

    else {
      // oid does not exist
      //
      // response packet - object not found
      pduRequest.type = SNMP_PDU_RESPONSE;
      pduRequest.error = SNMP_ERR_NO_SUCH_NAME;
    }
    Agentuino.responsePdu(&pduRequest);
  } 
  else {
    pduRequest.type = SNMP_PDU_RESPONSE;
    pduRequest.error = SNMP_ERR_NO_SUCH_NAME;
    Serial.print("in final else loop before sending response");
    return;
    //Agentuino.responsePdu(&pduRequest);
  }
  //
  Agentuino.freePdu(&pduRequest);
  //
  //Serial << "UDP Packet Received End.." << " RAM:" << freeMemory() << endl;
}

void setup()
{
  Serial.begin(115200);
  Ethernet.begin(mac, ip);
  //
  api_status = Agentuino.begin();
  Serial.println("Starting up api status is ");
  Serial.println(api_status);
  //
  if ( api_status == SNMP_API_STAT_SUCCESS ) {
    //
    Agentuino.onPduReceive(pduReceived);
    //
    delay(10);
    //
    Serial << F("SNMP Agent Initalized...") << endl;
    //
    return;
  }
  //
  delay(10);
  //
  Serial << F("SNMP Agent Initalization Problem...") << status << endl;
}

void loop()
{
  // listen/handle for incoming SNMP requests
  //Serial.println("listen start");
  Agentuino.listen();
  //Serial.println("listen end");
  //
  // sysUpTime - The time (in hundredths of a second) since
  // the network management portion of the system was last
  // re-initialized.
  if ( millis() - prevMillis > 1000 ) {
    // increment previous milliseconds
    prevMillis += 1000;
    //
    // increment up-time counter
    locUpTime += 100;
  }
}

