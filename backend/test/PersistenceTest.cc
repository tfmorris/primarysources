//
// Created by schaffert on 4/7/15.
//
#include "gtest.h"

#include <ctime>
#include <fstream>
#include <sstream>
#include <cppdb/frontend.h>
#include <boost/algorithm/string.hpp>

#include "Persistence.h"

class PersistenceTest : public ::testing::Test {

protected:
    std::string schema_path = "schema.sqlite.sql";

    cppdb::session sql;

    virtual void SetUp() {
        // setup a database and load the schema from the source directory;
        // test therefore needs to have the source directory as current working
        // directory
        sql.open("sqlite3:db=:memory:");

        std::ifstream schemaFile(schema_path);

        ASSERT_FALSE(schemaFile.fail());

        if (!schemaFile.fail()) {
            std::string statement;

            sql.begin();
            while (std::getline(schemaFile, statement, ';')) {
                boost::algorithm::trim(statement);
                if(statement.length() > 0) {
                    sql << statement << cppdb::exec;
                }
            }
            sql.commit();
        } else {
            std::cerr << "could not read schema.sqlite.sql; is the working "
                      << "directory configured properly?" << std::endl;
        }
    }

    virtual void TearDown() {
        sql.close();
    }

};

TEST_F(PersistenceTest, SchemaExists) {
    sql.begin();
    cppdb::result r = sql << "SELECT count(*) FROM statement" << cppdb::row;
    ASSERT_FALSE(r.empty());
    sql.commit();
}

TEST_F(PersistenceTest, AddGetSnak) {
    time_t rawtime = std::time(NULL);
    std::tm *t = std::gmtime(&rawtime);

    PropertyValue pvs[] = {
            PropertyValue("P123", Value("Hello, World!", "en")),
            PropertyValue("P124", Value("Q321")),
            PropertyValue("P125", Value(42.11, 11.32)),
            PropertyValue("P126", Value(*t, 9)),
    };

    Persistence p(sql, true);
    for (PropertyValue& pv : pvs) {
        sql.begin();
        int64_t id1 = p.addSnak(pv);
        int64_t id2 = p.getSnakID(pv);
        PropertyValue pvt = p.getSnak(id1);
        sql.commit();

        ASSERT_EQ(id1, id2);
        ASSERT_EQ(pv.getProperty(), pvt.getProperty());
    }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}