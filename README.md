# mymetrics

> UDF library of string metrics for MySQL 5

Available metrics:

- [Levenshtein distance](http://en.wikipedia.org/wiki/Levenshtein_distance)
- [Double Metaphone](http://en.wikipedia.org/wiki/Metaphone#Double_Metaphone)
- [Jaro-Winkler distance](http://en.wikipedia.org/wiki/Jaro%E2%80%93Winkler_distance)
- [Dice coefficient](http://en.wikipedia.org/wiki/S%C3%B8rensen%E2%80%93Dice_coefficient)

Features:

- supports UTF-8 strings
- very fast (especially in comparison with stored functions)
- easy to install and use

## How to install

Mymetrics is tested on Ubuntu 12.04 LTS x64, MySQL Community Server 5.6.17, cmake 2.8.9, gcc 4.6.3.

`cd` to project root directory and
```bash
mkdir build && cd build
cmake ..
make install
mysql < ../declare.sql
```

## How to use
```mysql
mysql> select levenshtein("ООО Рога и копыта", "Рога и копыта, ООО");
+------------------------------------------------------------------------------------+
| levenshtein("ООО Рога и копыта", "Рога и копыта, ООО")                             |
+------------------------------------------------------------------------------------+
|                                                                                  9 |
+------------------------------------------------------------------------------------+
1 row in set (0.00 sec)

mysql> select double_metaphone_eq("mère", "mer");
+-------------------------------------+
| double_metaphone_eq("mère", "mer")  |
+-------------------------------------+
|                                   1 |
+-------------------------------------+
1 row in set (0.00 sec)

mysql> select jaro_winkler("ООО Рага и копыта", "Рога и копыта, ООО");
+-------------------------------------------------------------------------------------+
| jaro_winkler("ООО Рага и копыта", "Рога и копыта, ООО")                             |
+-------------------------------------------------------------------------------------+
|                                                                  0.7007709066532596 |
+-------------------------------------------------------------------------------------+
1 row in set (0.00 sec)

mysql> select dice("ООО Рага и копыта", "Рога и копыта, ООО");
+-----------------------------------------------------------------------------+
| dice("ООО Рага и копыта", "Рога и копыта, ООО")                             |
+-----------------------------------------------------------------------------+
|                                                          0.7096774193548387 |
+-----------------------------------------------------------------------------+
1 row in set (0.00 sec)
```
