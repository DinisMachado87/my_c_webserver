#include "StrViewMap.hpp"
#include <gtest/gtest.h>

//  insert + find (SENSITIVE)

TEST(StrViewMap, FindEmpty)
{
	StrViewMap map;
	EXPECT_EQ(NULL, map.find("key"));
}

TEST(StrViewMap, InsertAndFind)
{
	StrViewMap map;
	map.insert("Host", "localhost");
	EXPECT_TRUE(map.find("Host") != NULL);
	EXPECT_EQ(StrView("localhost"), *map.find("Host"));
}

TEST(StrViewMap, FindMiss)
{
	StrViewMap map;
	map.insert("Host", "localhost");
	EXPECT_EQ(NULL, map.find("host"));
}

TEST(StrViewMap, InsertDuplicateKeepsAll)
{
	StrViewMap map;
	map.insert("X", "1");
	map.insert("X", "2");
	EXPECT_EQ(2u, map.size());
	EXPECT_EQ(StrView("1"), *map.find("X"));
}

//  find INSENSITIVE

TEST(StrViewMap, FindINSENSITIVECaseMatch)
{
	StrViewMap map;
	map.insert("Content-Type", "text/html");
	EXPECT_TRUE(map.find("content-type", StrViewMap::INSENSITIVE) != NULL);
	EXPECT_TRUE(map.find("CONTENT-TYPE", StrViewMap::INSENSITIVE) != NULL);
	EXPECT_TRUE(map.find("Content-Type", StrViewMap::INSENSITIVE) != NULL);
}

TEST(StrViewMap, FindINSENSITIVEMiss)
{
	StrViewMap map;
	map.insert("Host", "x");
	EXPECT_EQ(NULL, map.find("Content-Type", StrViewMap::INSENSITIVE));
}

TEST(StrViewMap, FindINSENSITIVEDifferentLength)
{
	StrViewMap map;
	map.insert("Host", "x");
	EXPECT_EQ(NULL, map.find("Hosts", StrViewMap::INSENSITIVE));
	EXPECT_EQ(NULL, map.find("Hos", StrViewMap::INSENSITIVE));
}

//  set

TEST(StrViewMap, SetInsertsNew)
{
	StrViewMap map;
	map.set("Host", "a");
	EXPECT_EQ(1u, map.size());
	EXPECT_EQ(StrView("a"), *map.find("host", StrViewMap::INSENSITIVE));
}

TEST(StrViewMap, SetReplacesExisting)
{
	StrViewMap map;
	map.set("Host", "old");
	map.set("host", "new");
	EXPECT_EQ(1u, map.size());
	EXPECT_EQ(StrView("new"), *map.find("Host", StrViewMap::INSENSITIVE));
}

TEST(StrViewMap, SetSENSITIVEDoesNotReplace)
{
	StrViewMap map;
	map.set("Host", "old", StrViewMap::SENSITIVE);
	map.set("host", "new", StrViewMap::SENSITIVE);
	EXPECT_EQ(2u, map.size());
}

TEST(StrViewMap, SetPreservesOtherEntries)
{
	StrViewMap map;
	map.set("A", "1");
	map.set("B", "2");
	map.set("a", "replaced");
	EXPECT_EQ(2u, map.size());
	EXPECT_EQ(StrView("replaced"), *map.find("A", StrViewMap::INSENSITIVE));
	EXPECT_EQ(StrView("2"), *map.find("B", StrViewMap::INSENSITIVE));
}

//  setIfMissing

TEST(StrViewMap, SetIfMissingInsertsNew)
{
	StrViewMap map;
	map.setIfMissing("Host", "val");
	EXPECT_EQ(1u, map.size());
	EXPECT_EQ(StrView("val"), *map.find("Host", StrViewMap::INSENSITIVE));
}

TEST(StrViewMap, SetIfMissingSkipsExisting)
{
	StrViewMap map;
	map.set("Host", "original");
	map.setIfMissing("host", "ignored");
	EXPECT_EQ(1u, map.size());
	EXPECT_EQ(StrView("original"), *map.find("Host", StrViewMap::INSENSITIVE));
}

TEST(StrViewMap, SetIfMissingSENSITIVEDistinguishesCase)
{
	StrViewMap map;
	map.set("Host", "a", StrViewMap::SENSITIVE);
	map.setIfMissing("host", "b", StrViewMap::SENSITIVE);
	EXPECT_EQ(2u, map.size());
}

//  at

TEST(StrViewMap, AtReturnsInOrder)
{
	StrViewMap map;
	map.insert("A", "1");
	map.insert("B", "2");
	map.insert("C", "3");
	EXPECT_EQ(StrView("A"), map.at(0).first);
	EXPECT_EQ(StrView("B"), map.at(1).first);
	EXPECT_EQ(StrView("C"), map.at(2).first);
}

//  copy / assign

TEST(StrViewMap, CopyConstructor)
{
	StrViewMap a;
	a.insert("K", "V");
	StrViewMap b(a);
	EXPECT_EQ(1u, b.size());
	EXPECT_EQ(StrView("V"), *b.find("K"));
}

TEST(StrViewMap, AssignmentOperator)
{
	StrViewMap a;
	a.insert("K", "V");
	StrViewMap b;
	b = a;
	EXPECT_EQ(1u, b.size());
	b.set("K", "X", StrViewMap::SENSITIVE);
	EXPECT_EQ(StrView("V"), *a.find("K"));
	EXPECT_EQ(StrView("X"), *b.find("K"));
}
