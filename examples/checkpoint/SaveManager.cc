#include "SaveManager.hh"
#include <iostream>
//////////////////////// Class Features ////////////////////////////
SaveManager::loadCrashes(vector<tuple<
			 Nullable<Real>,
			 Nullable<Real>,
			 map<const string, tuple<bool,Nullable<Real>,string>>>> &data,
			 
			 Nullable<Real>(*time)()){
  time_func = time;
  if(!directory_set){
    cout << "WARNING: SaveManager: directory not specified, using default of ./" << endl;
  }
  have_read = true;
}
