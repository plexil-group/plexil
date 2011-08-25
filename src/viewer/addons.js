/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
*
* By Madan, Isaac A.
*/

// JavaScript Document
var myTokenFileName = "../rawPlanTokens.js";
var showGeneratedNodes = provideGenCookie();
var showExpanded = provideLineCookie();

//to show the plan currently being viewed
var numtimes = 0;
$(document).ready(callReadyFunc);
function callReadyFunc() { 
	if(numtimes < 1) {
		$('#mod').append(tellIfDefault + '<div="addplan"><input type="text" id="planname" class="newhidden" /><button id="planstartup" class="newhidden">Add plan</button><button id="defaultplan" class="newhidden">Latest run</button></div>');
	}
	numtimes++;
}

/** COOKIES
	* showGenCookie - boolean, toggle generated nodes
	* showLineCookie - boolean, toggle timeline/expanded
	* showFileCookie - string, file name of plan, deprecated and no longer in use
	* showPixelsCookie - parsed as int, perserve getPixelsPerTimeIncrement() - value from UI, defined in main
	* showHeightCookie - parsed as int, perserve getTokenHeight() - value from UI, defined in main
	* showScaleCookie - parsed as int, perserve getScaling() - value from UI, defined in main
	* showCustomCookie - parsed as string, specific nodes to hide, defined in detailsBox
**/

function provideGenCookie() {
	var temp = getCookie("showGenCookie");
	if(temp != null && temp != "") {
		return temp;
	}
	else {
		setCookie("showGenCookie","false",365);
		var newtemp = getCookie("showGenCookie");
		return newtemp;
	}
}

function provideLineCookie() {
	var temp = getCookie("showLineCookie");
	if(temp != null && temp != "") {
		return temp;
	}
	else {
		setCookie("showLineCookie","true",365);
		var newtemp = getCookie("showLineCookie");
		return newtemp;
	}
}

/** cookie setters, accessors, deletors **/

function setCookie(c_name,value,exdays)
{
var exdate=new Date();
exdate.setDate(exdate.getDate() + exdays);
var c_value=escape(value) + ((exdays==null) ? "" : "; expires="+exdate.toUTCString());
document.cookie=c_name + "=" + c_value;
}

function getCookie(c_name)
{
var i,x,y,ARRcookies=document.cookie.split(";");
for (i=0;i<ARRcookies.length;i++)
{
  x=ARRcookies[i].substr(0,ARRcookies[i].indexOf("="));
  y=ARRcookies[i].substr(ARRcookies[i].indexOf("=")+1);
  x=x.replace(/^\s+|\s+$/g,"");
  if (x==c_name)
    {
    return unescape(y);
    }
  }
}

function deleteCookie(name) {
document.cookie = name +
'=; expires=Thu, 01-Jan-70 00:00:01 GMT;';
}

function deleteAllCookies() {
	deleteCookie("showGenCookie");
	deleteCookie("showLineCookie");
 	deleteCookie("showFileCookie");
 	deleteCookie("showPixelsCookie");
	deleteCookie("showHeightCookie");
	deleteCookie("showScaleCookie");
	deleteCookie("showCustomCookie");
}

function initializeCustomNodesArray() {
	customNodesArray = unpackCSVString(getCookie("showCustomCookie"));
	for(var i = 0; i < customNodesArray.length; i++) {
		while(customNodesArray[i].charAt(0) == "\n") customNodesArray[i] = customNodesArray[i].substring(1);
		while(customNodesArray[i].charAt(0) == " ") customNodesArray[i] = customNodesArray[i].substring(1);
		while(customNodesArray[i].charAt(customNodesArray[i].length-1) == " ") customNodesArray[i] = customNodesArray[i].substring(0, customNodesArray[i].length-1);
	}
}

/** determines if a custom node matches a token value **/
function isCustomNode(temp, temp2, temp3) {
	var masterDecision = false;
	var isExactNode = doExactNode(temp, temp2, temp3);
	var isRegularExpression = doRegularExpression(temp,temp2,temp3);
	if(isExactNode || isRegularExpression) masterDecision = true;
	return masterDecision;
}

/** checks custom node specifications for exact matches **/
function doExactNode(temp, temp2, temp3) {
	var isCustom = false;
	if(getCookie("showCustomCookie") == null || getCookie("showCustomCookie") =='') return isCustom;
	else initializeCustomNodesArray();
	for(var i = 0; i < customNodesArray.length; i++) {
		if((customNodesArray[i].indexOf('*') == -1) && (customNodesArray[i].indexOf('+') == -1) && (customNodesArray[i].indexOf('?') == -1)) {
			if(getCookie("showLineCookie") == "false") {
				if(((temp == customNodesArray[i]) && (temp.length == customNodesArray[i].length))) 
					isCustom = true;
			}
			else {
				if(((temp2 == customNodesArray[i]) && (temp2.length == customNodesArray[i].length)))
					isCustom = true;
			}
		}
	}
	return isCustom;
}

/** checks custom node specifications for wildcard matches **/
function doRegularExpression(temp,temp2,temp3) {
	var isRegExp = false;
	for(var i = 0; i < customNodesArray.length; i++) {
		if(customNodesArray[i].indexOf('*') != -1) {
			if(getCookie("showLineCookie") == "false")
				isRegExp = handleRegularExpression(customNodesArray[i], temp, temp, temp3);
			else
				isRegExp = handleRegularExpression(customNodesArray[i], temp, temp2, temp3);
			if(isRegExp) return isRegExp;
		}
	}
	return isRegExp;
}

/** handles regular expression/wildcard * in custom node specifications **/
function handleRegularExpression(string, temp, temp2, temp3) {
	var stars = new Array();
	var finalBool = true;
	for(var i = 0; i < string.length; i++) {
		if(string.charAt(i) == '*') {
			stars.push(i);
		}
	}
	var start = 0;
	var constructStrings = new Array();
	for(var i = 0; i < stars.length; i++) {
		var newstring = string.substring(start, stars[i]);
		start = stars[i]+1;
		constructStrings.push(newstring);
	}
	if(start != string.length) {
		var newstring = string.substring(start);
		constructStrings.push(newstring);
	}
	var tempBools = new Array();
	//string must contain all construct strings for the function to return true, indicating a target node
	if(temp2.length < string.length) tempBools.push(false);
	for(var i = 0; i < constructStrings.length; i++) {
		if((temp2.indexOf(constructStrings[i]) != -1)) {
			tempBools.push(true);
		} 
		else tempBools.push(false);
	}
	for(var i = 0; i < constructStrings.length-1; i++) {
		var tempString = constructStrings[i] + constructStrings[i+1];
		if(temp2.indexOf(tempString) != -1)
			tempBools.push(false);
	}
	if(string[string.length-1] != '*') {
		if(string[string.length-1] != temp2[temp2.length-1])
			tempBools.push(false);
	}
	for(var i = 0; i < tempBools.length; i++) {
		if(tempBools[i] == false) finalBool = false;
	}
	return finalBool;
}

/** lists nodes that are hidden **/
function doCustomUnhide(temp, temp2) {
	var isInc = false;
	for(var i = 0; i < customNodesUnhideArray.length; i++) {
		if(customNodesUnhideArray[i] == temp2) isInc = true;
	}
	if(!isInc) customNodesUnhideArray.push(temp2);
	$('#gantt').append([
								 '<div id="customNodesUnhideBox">',
								 '<table border="0">',
								 '<tr>',
								 '<td width="150"><strong>Node name</td>',
								 '</tr>',
								 ].join(''));
	for(var i = 0; i < customNodesUnhideArray.length; i++) {
		$('#customNodesUnhideBox').append([
									 '<tr>',
									 '<td>&nbsp;',
									 customNodesUnhideArray[i],
									 '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;',
									 '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>',
									 '</tr>',
									 ].join(''));
	}
	$('#customNodesUnhideBox').append([
								 '</table>',
								 '</div>',
								 ].join(''));
	$('#customNodesUnhideBox').dialog({
								autoOpen:false,
								height:250,
								width:400,
								title:'Hidden nodes',
								});
	$('#customNodesUnhideBox').dialog("close");
}

/** return value to main for checked scale radio box **/
function manageRadioScaleChecked(boxname) {
	var desiredVal = getCookie("showScaleCookie");
	if(desiredVal == null) {
		if(boxname == 4)
			return "checked = \"checked\"";
		else
			return " ";
	}
	else if(desiredVal == "1000" && boxname == 1)
		return "checked = \"checked\"";
	else if(desiredVal == "100" && boxname == 2)
		return "checked = \"checked\"";
	else if(desiredVal == "10" && boxname == 3)
		return "checked = \"checked\"";
	else if(desiredVal == "1" && boxname == 4)
		return "checked = \"checked\"";
	else
		return " ";
}

/** alerter for debugging **/
var alerter = 0;
function alertonce(string) {
	if(alerter == 0) {
		alert(string);
	}
	alerter++;
}