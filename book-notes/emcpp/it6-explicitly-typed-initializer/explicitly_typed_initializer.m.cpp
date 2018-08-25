#include <iostream>
#include <string>
#include <vector>
#include <boost/type_index.hpp>

using namespace std;

vector<bool> buildVector() {
  vector<bool> bs;
  bs.push_back(false);
  bs.push_back(false);
  bs.push_back(false);
  bs.push_back(false);
  bs.push_back(false);
  bs.push_back(true);
  return bs;
}

int main() {
  auto highPriority = buildVector()[5];
  cout << "type: " << boost::typeindex::type_id_with_cvr<decltype(highPriority)>() << "\n";
  
  if (highPriority) {     // undefined behavior: reference to deleted temporary
    cout << "high priority\n";
  } else {
    cout << "low priority\n";
  }

  // instead, do
  auto highPriorityGood = static_cast<bool>(buildVector()[5]);
  if (highPriorityGood) {     // all good
    cout << "high priority\n";
  } else {
    cout << "low priority\n";
  }

  return 0;
}