/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XMLUtils.hh"
#include "Debug.hh"

/**
 * @author Conor McGann
 */

#include <fstream>
#include <sstream>

namespace PLEXIL {

  LabelStr extractData(const TiXmlElement& configData, const LabelStr& argName){
    const char * data = configData.Attribute(argName.c_str());
    checkError(data != NULL, "No attribute '" << argName.toString() << "' in " << configData);
    return data;
  }

  TiXmlElement* initXml(const std::string& xmlStr) {
    std::istringstream is(xmlStr);
    TiXmlElement* root = new TiXmlElement("");
    is >> (*root);
    return root;
  }

  const char* getTextChild (const TiXmlElement& element) {
    check_error(element.FirstChild(), "FirstChild is empty.");
    //    check_error(element.FirstChild()->ToComment(), "FirstChild->ToComment is empty");
    check_error(element.FirstChild()->ToText(), "FirstChild->ToText is empty.");
    check_error(element.FirstChild()->ToText()->Value(), "FirstChild->ToText->Value is empty.");

    return element.FirstChild()->ToText()->Value();
  }

  bool isNumber(const char* data){
    double dummy;
    return isNumber(data, dummy);
  }

  bool isNumber(const char* data, double& value){
    char * pEnd;
    value = strtod (data, &pEnd);

    // If successful then quit now
    if(pEnd != data)
      return true;
    else
      return false;
  }
}
