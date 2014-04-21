DROP FUNCTION levenshtein;
DROP FUNCTION double_metaphone_eq;
DROP FUNCTION jaro_winkler;
DROP FUNCTION dice;

CREATE FUNCTION levenshtein RETURNS INTEGER SONAME 'libmymetrics.so';
CREATE FUNCTION double_metaphone_eq RETURNS INTEGER SONAME 'libmymetrics.so';
CREATE FUNCTION jaro_winkler RETURNS REAL SONAME 'libmymetrics.so';
CREATE FUNCTION dice RETURNS REAL SONAME 'libmymetrics.so';
