rm -rf mvbcoins
mkdir mvbcoins
cp makefile mvbcoins/
cp server mvbcoins/
cp tester mvbcoins/
cp readme.txt mvbcoins/
mkdir mvbcoins/src
cp src/*.cpp mvbcoins/src
cp src/*.h mvbcoins/src
rm -rf mvbcoins.zip
zip -r mvbcoins.zip mvbcoins
