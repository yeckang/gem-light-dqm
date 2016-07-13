
#define PORT 3306
#include <mysql/mysql.h>
#include <Python.h>

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <array>
#include <memory>
#include <unordered_map>

using namespace std;

//Queries that return multiple values
vector<char*> manyDBQuery(MYSQL * Database, std::string m_Query)
{
  vector<char*> result;
  vector<char*> null_return{"0"};
  const char * manyQuery = m_Query.c_str();

  int qresult = mysql_query(Database,manyQuery);
  if (qresult) {
    std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
    return null_return;
  }
  else
    if(DEBUG) std::cout << "MySQL query success: " << manyQuery << std::endl;

  MYSQL_RES* res = mysql_use_result(Database);
  MYSQL_ROW row;

  while (((row=mysql_fetch_row(res)) !=NULL))
    {
      result.push_back(row[0]);
    }

  mysql_free_result(res);

  return result;
}

  
//Queries that should return a single value
char* simpleDBQuery(MYSQL * Database, std::string m_Query)
{
  char * null_return = "0";
  const char * query = m_Query.c_str();
  try {
    int rv = mysql_query(Database,query);
    if (rv) {
      std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
      return null_return;
    }
    else
      if(DEBUG) std::cout << "MySQL query success: " << m_Query << std::endl;
    MYSQL_RES* res = mysql_use_result(Database);
    MYSQL_ROW  row = mysql_fetch_row(res);
    if (row == 0) {
      std::cout << "Query result " << m_Query << " empty" << std::endl;
      return null_return;
    }
    char* retval = row[0];
    mysql_free_result(res);
    return retval;
  } catch (std::exception& e) {
    std::cout << "simpleDBQuery caught std::exception " << e.what() << std::endl;
  }
    
}

unsigned int getRunNumber(MYSQL* Database)
{
  
  std::string    setup = "teststand";
  std::string   period = "2016T";
  std::string location = "TIF";
  std::string lastRunNumberQuery = "SELECT Number FROM ldqm_db_run WHERE Station LIKE '";
  lastRunNumberQuery += location;
  lastRunNumberQuery += "' ORDER BY id DESC LIMIT 1;";
  const char * query = lastRunNumberQuery.c_str();
    
  try {
    int rv = mysql_query(Database,query);
    if (rv)
      std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
    else
      if(DEBUG) std::cout << "[getRunNumber]: MySQL query success: " << lastRunNumberQuery << std::endl;

    MYSQL_RES* res = mysql_use_result(Database);
    MYSQL_ROW  row = mysql_fetch_row(res);
    if (row == 0) {
      std::string errMsg = "[getRunNumber]: Query result " + lastRunNumberQuery + " empty";
      std::cout <<"[getRunNumber]: " << errMsg << std::endl;
    }
    unsigned int retval = strtoul(row[0],0,0);
    mysql_free_result(res);
      
    if (DEBUG) std::cout << "[getRunNumber]: New run number is: " << retval << std::endl;
    return retval;
  } catch (std::exception& e) {
    std::cout << "[getRunNumber]: caught std::exception " << e.what() << std::endl;
  }
}

unsigned int getChipIDFromID(MYSQL * Database, unsigned int db_id)
{
  std::string m_Query = "SELECT ChipID FROM ldqm_db_vfat WHERE id LIKE ";
  m_Query += std::to_string((long long int)db_id);
  const char * query = m_Query.c_str();
  try {
    int rv = mysql_query(Database,query);
    if (rv)
      std::cout << "MySQL query error: " << std::string(mysql_error(Database)) << std::endl;
    else
      if(DEBUG) std::cout << "[getChipIDFromID]: MySQL query success: " << m_Query << std::endl;
    MYSQL_RES* res = mysql_use_result(Database);
    MYSQL_ROW  row = mysql_fetch_row(res);
    if (row == 0) {
      std::string errMsg = "[getChipIDFromID]: Query result " + m_Query + " empty";
      std::cout << errMsg << std::endl;
      return NULL;
    }
    unsigned int retval = strtoul(row[0],0,0);
    mysql_free_result(res);
      
    if (DEBUG) std::cout << "[getChipIDFromID]: ChipID is: " << std::hex << retval << std::dec << std::endl;
    return retval;
  } catch (std::exception& e) {
    std::cout << "[getChipIDFromID]: caught std::exception " << e.what() << std::endl;
  }
}

unsigned int getVFATChipID(MYSQL * Database, std::string AMCboardid, std::string GEBchamberid, int slot) {

  std::string AMCdbidQuery  = "SELECT id FROM ldqm_db_amc WHERE BoardID LIKE '";
  AMCdbidQuery += AMCboardid;
  AMCdbidQuery += "'";
  int AMCdbid = atoi(simpleDBQuery(Database,AMCdbidQuery));
  if (!AMCdbid)
    return 0;
  std::cout << "AMCdbid: " << AMCdbid << std::endl;

  std::string GEBdbidsQuery  = "SELECT geb_id FROM ldqm_db_amc_gebs WHERE amc_id LIKE ";
  GEBdbidsQuery += std::to_string((long long int)AMCdbid);
  vector<char*> GEBdbids = manyDBQuery(Database,GEBdbidsQuery);
    
  int correctGEBID = 0;
  for (int id = 0; id < GEBdbids.size(); id++) {
    int currentGEBid = atoi(GEBdbids[id]);
    cout << "GEBdbids: " << currentGEBid << endl;
    std::string GEBchambermatchQuery = "SELECT ChamberID FROM ldqm_db_geb WHERE id LIKE ";
    GEBchambermatchQuery += std::to_string((long long int)atoi(GEBdbids[id]));
    char* dbchamberid = simpleDBQuery(Database,GEBchambermatchQuery);
    std::string chamberiddb = dbchamberid;
    if (chamberiddb.compare(GEBchamberid)) {
      std::cout << "Located correct GEB: " << chamberiddb << std::endl;
      correctGEBID = currentGEBid;
      break;
    }
  }

  if (correctGEBID==0) {
    std::cout << "Could not locate correct GEB" << std::endl;
    return 0;
  }

  std::cout << correctGEBID << std::endl;
    
  std::string GEBVFATsQuery = "SELECT vfat_id FROM ldqm_db_geb_vfats WHERE geb_id LIKE ";
  GEBVFATsQuery += std::to_string((long long int)correctGEBID);
  vector<char*> correctGEBVFATs = manyDBQuery(Database,GEBVFATsQuery);

  int correctVFATID = 0;
  for (int vf=0; vf<correctGEBVFATs.size();vf++) {
    int currentVFATID = atoi(correctGEBVFATs[vf]);
    std::cout << currentVFATID << std::endl;

    std::string VFATSlotQuery = "SELECT Slot FROM ldqm_db_vfat WHERE id LIKE ";
    VFATSlotQuery += std::to_string((long long int)currentVFATID);
    int currentSlot = atoi(simpleDBQuery(Database,VFATSlotQuery));
    if (currentSlot == slot) {
      correctVFATID = currentVFATID;
      break;
    }   
  }

  if (correctVFATID != 0) {
    int ChipID = strtol(simpleDBQuery(Database,"SELECT ChipID FROM ldqm_db_vfat WHERE id LIKE "+std::to_string((long long int)correctVFATID)),NULL,16);
    std::cout << std::hex << ChipID << std::endl;
    return ChipID;
  }
}
