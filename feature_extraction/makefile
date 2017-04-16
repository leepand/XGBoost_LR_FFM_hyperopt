all:ffm_feature_extract svm_feature_extract
ffm_feature_extract:ffm_feature_extract_new.cpp
	g++ -std=c++11 -o ffm_feature_extract ffm_feature_extract_new.cpp -lpthread
svm_feature_extract:svm_feature_extract_new.cpp
	g++ -std=c++11 -o svm_feature_extract svm_feature_extract_new.cpp -lpthread
clean:
	rm svm_feature_extract svm_features.txt svm_features.txt.uid svm_features.txt.fid
