all: predict_ffm_f predict_svm_f
predict_ffm_f:predict_ffm_feature_extract_new.cpp
	g++ -std=c++11 -o predict_ffm_f predict_ffm_feature_extract_new.cpp -lpthread
predict_svm_f:predict_svm_feature_extract_new.cpp
	g++ -std=c++11 -o predict_svm_f predict_svm_feature_extract_new.cpp -lpthread
clean:
	rm predict_ffm_f predict_svm_f

