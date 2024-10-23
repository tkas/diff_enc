#NIR
#-------------------------------------------------
IN_FILE=./test_dataformat/Bjurbole_Side1_NIR_35ms_1A.mat.dat

./diff_bpp 617 489 85 <${IN_FILE} 2>/tmp/${IN_FILE}.txt >${IN_FILE}.diff.img

