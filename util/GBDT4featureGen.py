# coding: utf-8

import hashlib
def hashstr(str, nr_bins):
    return int(hashlib.md5(str.encode('utf8')).hexdigest(), 16)%(nr_bins-1)+1
'''
def gen_hashed_fm_feats(feats, nr_bins,libsvmformat=False):
    if libsvmformat:
        feats = ['{1}:1'.format(field-1, hashstr(feat, nr_bins)) for (field, feat) in feats]
    else:
        feats = ['{0}:{1}:1'.format(field-1, hashstr(feat, nr_bins)) for (field, feat) in feats]
    return feats
'''
def gen_hashed_fm_feats(feats, nr_bins,libsvmformat=False):
    if libsvmformat:
        f_sort=[]
        for (field, feat) in feats:
            f_sort.append(int(hashstr(feat, nr_bins))) 
        u=sorted(set(f_sort))
        #print [(i,j) for (i,j) in  enumerate(u)]
        feats = ['{1}:1'.format(i,j) for (i,j) in  enumerate(u)]
    else:
        feats = ['{0}:{1}:1'.format(field-1, hashstr(feat, nr_bins)) for (field, feat) in feats]
    return feats
def save_ffm_libsvm_features_GBDT_gen(HashLibsvmFile,HashFFMFile,leaf_tolist,label_tolist,nr_bins):
    with open(HashLibsvmFile, 'w') as f,open(HashFFMFile, 'w') as fo_ffm:
        print ('Gen(GBDT) features beginning...')
        for sample in range(len(leaf_tolist)):
            feat_leaf_ffm=[]
            feat_leaf_libsvm=[]
            for row,feat in enumerate(leaf_tolist[sample]):
                field=row+1
                feat_leaf_ffm.append((field, 'GBDT'+str(row)+":"+str(feat)) )
            feats_ffm = gen_hashed_fm_feats(feat_leaf_ffm,nr_bins)
            feats_libsvm = gen_hashed_fm_feats(feat_leaf_ffm,nr_bins,libsvmformat=True)
            fo_ffm.write(str(label_tolist[sample]) + ' ' + ' '.join(feats_ffm) + '\n')
            f.write(str(label_tolist[sample]) + ' ' + ' '.join(feats_libsvm) + '\n')
    print ('Gen(GBDT) features Done!')
            
'''      
with open('hash_feat_leaf.libsvm', 'w') as f,open('hash_feat_leaf.ffm', 'w') as fo_ffm:
    for sample in range(len(leafindex.tolist())):
        feat_leaf_ffm=[]
        feat_leaf_libsvm=[]
        for row,feat in enumerate(leafindex.tolist()[sample]):
            field=row+1
            feat_leaf_ffm.append((field, 'GBDT'+str(row)+":"+str(feat)) )
        feats_ffm = gen_hashed_fm_feats(feat_leaf_ffm,100)
        feats_libsvm = gen_hashed_fm_feats(feat_leaf_ffm,100,libsvmformat=True)
        fo_ffm.write(str(dtest.get_label().tolist()[sample]) + ' ' + ' '.join(feats_ffm) + '\n')
        f.write(str(dtest.get_label().tolist()[sample]) + ' ' + ' '.join(feats_libsvm) + '\n')
'''     

