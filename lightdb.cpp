#include "LightDB/db.h"
#include "LightDB/write_batch.h"
#include <cassert>
#include <iostream>

using namespace std;
using namespace LightDB;

int main() {
    LightDB::DB *db;
    LightDB::Status status = LightDB::DB::Open("testdb", &db);
    assert(status.ok());

    WriteBatch write;
    for (int i = 0; i < 50; i++) {
        write.Put("hello"+to_string(i), "hsfa"+to_string(i));
    }
    status = db->Write(&write);
    assert(status.ok());
    for (int i = 0; i < 50; i++) {
        string tmp;
        status = db->Get("hello"+to_string(i), &tmp);
        assert(status.ok());
        cout << tmp << endl;
    }

    delete db;
    return 0;
}
