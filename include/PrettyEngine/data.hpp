#pragma once

#include <PrettyEngine/debug.hpp>
#include <PrettyEngine/utils.hpp>

#include <cstring>
#include <sqlite3/sqlite3.h>

#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>

namespace PrettyEngine {
	static int SQLite3Callback(void* notUsed, int argc, char** argv, char** colName) {
		int i;
	    for(i=0; i<argc; i++){
    	  printf("%s = %s\n", colName[i], argv[i] ? argv[i] : "NULL");
    	}
    	printf("\n");
    	return 0;
	}

	struct SQLBlobData {
		std::vector<unsigned char> data;
		int bytes;
	};

	class DataBase {
	public:
		DataBase(std::string fileName) {
			if (sqlite3_open(fileName.c_str(), &this->db) != 0) {
				DebugLog(LOG_ERROR, "Failed to open database file: " << fileName, true);
			}
		}

		~DataBase() {
			sqlite3_close(this->db);
		}

	public:
		/// Execute only the command, but do not query any result
		void ExecuteSQL(std::string sqlCommand) {
			char* errMsg = nullptr;

			if (sqlite3_exec(this->db, sqlCommand.c_str(), SQLite3Callback, NULL, &errMsg) != 0) {
				DebugLog(LOG_ERROR, "SQL Command failed: " << sqlCommand, true);
				DebugLog(LOG_ERROR, "SQL Error: " << errMsg, true);
			}

			sqlite3_free(errMsg);
		}

		std::vector<int> QuerySQLInt(std::string sqlCommand) {
			std::vector<int> out;

			sqlite3_stmt *stmt;
			
			auto rc = sqlite3_prepare_v2(this->db, sqlCommand.c_str(), -1, &stmt, NULL);

			while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        		int numColumns = sqlite3_column_count(stmt);
        		
        		for(int column = 0; column < numColumns; column++) {
    				if (sqlite3_column_type(stmt, column) == SQLITE_INTEGER) {
    					out.push_back(sqlite3_column_int(stmt, column));
    				}
        		}
        	}

        	if (rc != SQLITE_DONE) {
    			DebugLog(LOG_ERROR, "SQL error during stepping: " << sqlite3_errmsg(this->db), true);
			}

			sqlite3_finalize(stmt);

			return out;
		}

		std::vector<double> QuerySQLDouble(std::string sqlCommand) {
			std::vector<double> out;

			sqlite3_stmt *stmt;
			
			auto rc = sqlite3_prepare_v2(this->db, sqlCommand.c_str(), -1, &stmt, NULL);

			while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        		int numColumns = sqlite3_column_count(stmt);
        		
        		for(int column = 0; column < numColumns; column++) {
    				if (sqlite3_column_type(stmt, column) == SQLITE_FLOAT) {
    					out.push_back(sqlite3_column_double(stmt, column));
    				}
        		}
        	}

        	if (rc != SQLITE_DONE) {
    			DebugLog(LOG_ERROR, "SQL error during stepping: " << sqlite3_errmsg(this->db), true);
			}

			sqlite3_finalize(stmt);

			return out;
		}

		std::vector<std::string> QuerySQLText(std::string sqlCommand) {
			std::vector<std::string> out;

			sqlite3_stmt *stmt;
			
			auto rc = sqlite3_prepare_v2(this->db, sqlCommand.c_str(), -1, &stmt, NULL);

			while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        		int numColumns = sqlite3_column_count(stmt);
        		
        		for(int column = 0; column < numColumns; column++) {
    				if (sqlite3_column_type(stmt, column) == SQLITE_TEXT) {
    					std::stringstream ss;
    					ss << sqlite3_column_text(stmt, column);

    					out.push_back(ss.str());
    				}
        		}
        	}

        	if (rc != SQLITE_DONE) {
    			DebugLog(LOG_ERROR, "SQL error during stepping: " << sqlite3_errmsg(this->db), true);
			}

			sqlite3_finalize(stmt);

			return out;
		}

		std::vector<SQLBlobData> QuerySQLBlob(std::string sqlCommand) {
			std::vector<SQLBlobData> out;

			sqlite3_stmt *stmt;
			
			auto rc = sqlite3_prepare_v2(this->db, sqlCommand.c_str(), -1, &stmt, NULL);

			while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        		int numColumns = sqlite3_column_count(stmt);
        		
        		for(int column = 0; column < numColumns; column++) {
        			if (sqlite3_column_type(stmt, column) == SQLITE_BLOB) {
	    				const unsigned char* blobData = static_cast<const unsigned char*>(sqlite3_column_blob(stmt, column));
						int blobSize = sqlite3_column_bytes(stmt, column);

						std::vector<unsigned char> dataVector(blobData, blobData + blobSize);
				        for (int i = 0; i < blobSize; ++i) {
				            dataVector[i] = blobData[i];
				        }

				        SQLBlobData data;
				        data.data = dataVector;
				        data.bytes = blobSize; 
				        out.push_back(data);
			        }
        		}
        	}

        	if (rc != SQLITE_DONE) {
    			DebugLog(LOG_ERROR, "SQL error during stepping: " << sqlite3_errmsg(this->db), true);
			}

			sqlite3_finalize(stmt);

			return out;
		}

	private:
		sqlite3* db;
	};
}
