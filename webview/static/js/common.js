/*****************************************************
             PROJECT  : numaprof
             VERSION  : 2.3.0
             DATE     : 05/2017
             AUTHOR   : Valat Sébastien - CERN
             LICENSE  : CeCILL-C
*****************************************************/

/*******************  FUNCTION  *********************/
function logError(message)
{
	$( "<div>" ).text(message).appendTo( "#errors" );
}

/*******************  FUNCTION  *********************/
function valOfDefault(value,defaultValue)
{
	if (value == undefined)
		return defaultValue;
	else
		return value;
}

/*******************  FUNCTION  *********************/
function listToRange(elts)
{
	var start = -1;
	var last = -1;
	var out = [];
	
	//loop on all
	for (var i in elts)
	{
		if (start == -1)
		{
			start = +elts[i]
		} else if (elts[i] != last+1) {
			if (start != last)
				out.push(start+"-"+last);
			else
				out.push(start);
			start = +elts[i]
		}
		last = +elts[i]
	}

	//flush
	if (start != last)
		out.push(start+"-"+last);
	else
		out.push(start);
	
	//join
	return out.join(", ")
}
