///////////////////////////////////////////////////////////////////////
// PayLoad.h - application defined payload                           //
// ver 1.0                                                           //
// Vikrant Bhopatrao, CSE687 - Object Oriented Design, Spring 2018   //
///////////////////////////////////////////////////////////////////////

#include "PayLoad.h"
#include "../Utilities/StringUtilities/StringUtilities.h"

using namespace NoSqlDb;
using namespace XmlProcessing;

//----< test stub >----------------------------------------------------
#ifdef TEST_PAYLOAD

int main()
{
  Utilities::Title("Demonstrating Application Specific PayLoad class");
  Utilities::putline();

  using Sptr = std::shared_ptr<AbstractXmlElement>;

  Utilities::title("creating xml string from payload instance");
  Payload pl;
  pl.filepath("demo payload value");
  pl.categories().push_back("cat1");
  pl.categories().push_back("cat2");
  Sptr sPpl;
  pl.storePayload(sPpl, pl);
  XmlDocument doc(sPpl);
  std::cout << doc.toString();
  Utilities::putline();

  Utilities::title("creating payload instance from an XmlElement");
  Payload newPl = pl.restorePayload(sPpl);
  std::cout << "\n  payload value = " << pl.filepath();
  std::cout << "\n  payload categories:\n    ";
  for (auto cat : newPl.categories())
  {
    std::cout << cat << " ";
  }
  std::cout << "\n\n";
  pl.editPayload("../root");
  std::cout << "\n  payload value = " << pl.filepath();
  
  std::cout << "\n  Category Present = " << pl.match("header");
}
#endif
