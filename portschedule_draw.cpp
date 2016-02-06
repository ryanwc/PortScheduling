/**
 * This program produces a gnuplot file to draw a port schedule
*/

#include <cassert>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

template <class T>                                 // DZN VECTOR INPUT
istream& operator>>(istream& is, vector<T>& vec) {
	vec.clear();
	char c;
	is >> c;
	assert('[' == c);
	do {
		is >> skipws >> c;
		if (']' == c)
			break;
		is.putback(c);
		int n;
		is >> n;
		assert(is);
		vec.push_back(n);
		is >> c;
		if (',' != c)
			is.putback(c);
	} while(1);
	is >> skipws >> c;
	assert(';' == c);
	return is;
}

template <class T>                                 // DZN VECTOR OUTPUT
ostream& operator<<(ostream& os, vector<T>& vec) {
	os << '[';
	for (size_t i=0; i<vec.size(); ++i) {
		os << vec[i];
		if (vec.size()-1 == i)
			os << ']';
		else
			os << ", ";
	}
	return os;
}

class Data {
public:
	vector<int> endstack;   // 
	vector<int> eastend   ;   // = [4, 8, 2, 5, 4, 3, 3];
	vector<int> westend;   // = [0, 0, 4, 0, 6, 6, 0];
	vector<int> stack  ;   // = [0, 52, 0, 108, 21, 0, 143];
	vector<int> reclaim;   // = [16, 84, 23, 128, 40, 12, 155];
	vector<int> finish ;   // = [28, 108, 29, 143, 52, 21, 164];
	vector<int> which  ;   // = [1, 1, 2, 1, 1, 2, 1];
	int obj    ;   // = 407;
	
	typedef map<string, vector<int>* > Parser;
	Parser prs;
	
	Data() {                      // CONSTRUCTOR: FILL THE DATA + PARSER
		prs["endstack"] = &endstack;
		prs["eastend"] = &eastend;
		prs["westend"] = &westend;
		prs["stack"] = &stack;
		prs["reclaim"] = &reclaim;
		prs["finish"] = &finish;
		prs["which"] = &which;
		obj = -1;
	}
	void Parse(istream& is) {
		cout << "Parsing input\n";
		while (is) {
			string name;
			char c;
			is >> skipws >> name;
			typename Parser::iterator ii = prs.find(name);
			if (prs.end() != ii) {
				cout << name << ": " << flush;
				is >> skipws >> c;
				assert('=' == c);
				is >> (*ii->second);
				cout << "array [1.." << ii->second->size() << "] = " << (*ii->second) << ';' << endl;
			}
			else if ("obj" == name) {
				is >> skipws >> c;
				is >> obj >> skipws >> c;
			}
			else
				is.ignore(INT_MAX, ';');
		}
	}
	void Check() {
		const size_t nSize = endstack.size();
		for (Parser::iterator ii=prs.begin(); ii!=prs.end(); ++ii) {
			if (ii->second->size() != nSize) {
				cerr << "  SIZE OF VECTOR '" << ii->first << "' is different: " << ii->second->size();
				assert(0);
			}
		}
	}
	string OutputPlot(ostream& ofs, string sOutputPlt) {
		string sOutputFinal = sOutputPlt;
		sOutputFinal += ".ps";
		ofs << "set term postscript font \"sans,9\"" << "        # can set pdf as well \n";
		ofs << "set output \'" << sOutputFinal << "\'" << '\n';
		
		ofs << "set title \"Pad & Machine Schedule for solution with obj value " << obj << "\"\n";
		ofs << "set autoscale\n";
		ofs << "set xlabel \'Time\' offset 0,-1.5" << '\n';
		ofs << "set ylabel \'Pad position\'" << '\n';
		ofs << "set xtics out" << '\n';
		ofs << "set ytics out" << '\n';
		ofs << "unset colorbox" << '\n';
		ofs << "set border lw 0.5" << '\n';
		ofs << "set key outside above right horizontal" << '\n';
		ofs << "plot ";
// 		ofs << " '-' with boxxyerrorbars notitle linecolor rgb \"gray\" fill solid 0.75 ";
// 		ofs << ", '-' with boxxyerrorbars notitle linecolor rgb \"gray\" fill solid 0.25 ";
// 		ofs << ", '-' with boxxyerrorbars notitle linecolor rgb \"gray\" fill solid 0.5 ";
		ofs << " '-' with boxxyerrorbars notitle  fill pattern border ";
		ofs << ", '-' with boxxyerrorbars notitle  fill pattern border ";
		ofs << ", '-' with boxxyerrorbars notitle  fill pattern border ";
		ofs << ", '-' with boxxyerrorbars notitle lt -1 fill empty";
		ofs << ", '-' with labels font \"12\" offset 0.5,0.5 notitle ";       // using 1:2:8
		
		const int nr2 = *max_element(which.begin(), which.end());             // N RECLAIMERS
		
		for (int m=0; m<nr2; ++m) {
			ofs << ", '-' with linespoints ls " << (m+2) << " lw 6 title 'R_" << (m+1) << "'";
		}
		//		ofs << "\'\' using 3:xtic(1) title \'Stockpiles in dwell\', \'\' using 4:xtic(1) title \'Stockpiles being reclaimed\' axes x1y1";
		//		ofs << ", \'" << AllStockpilesFileName.str() << "\' u 5:xtic(1) with linespoints title \'Total Tonnage\' axes x1y2";
		ofs << '\n';
		
		ofs << "# Dwell: putting first to have the empty fill pattern: x y xlow xhigh ylow yhigh (color?)\n";
		for (int iS=0; iS<eastend.size(); ++iS) {
			ofs << "1 1 "
			<< endstack[iS] << ' '
			<< reclaim[iS] << ' '
			<< westend[iS] << ' ' << eastend[iS]
			<< '\n';
		}
		ofs << "end\n";
		ofs << "# Stacking: x y xlow xhigh ylow yhigh (color?)\n";
		for (int iS=0; iS<eastend.size(); ++iS) {
			ofs << "1 1 "
			<< stack[iS] << ' '
			<< endstack[iS] << ' '
			<< westend[iS] << ' ' << eastend[iS]
// 			<< ' ' << GetColor(iS+1)   /// Stacking part: cargo color, for cmp with FDS
			<< '\n';
		}
		ofs << "end\n";
		ofs << "# Reclaim: x y xlow xhigh ylow yhigh (color?)\n";
		for (int iS=0; iS<eastend.size(); ++iS) {
			ofs << "1 1 "
			<< reclaim[iS] << ' '
			<< finish[iS] << ' '
			<< westend[iS] << ' ' << eastend[iS]
			<< '\n';
		}
		ofs << "end\n";
		ofs << "# Whole boxes: x y xlow xhigh ylow yhigh (color?)\n";
		for (int iS=0; iS<eastend.size(); ++iS) {
			ofs << "1 1 "
			<< stack[iS] << ' '
			<< finish[iS] << ' '
			<< westend[iS] << ' ' << eastend[iS]
			<< ' ' << (iS+1)   /// Stacking part: cargo color, for cmp with FDS
			<< '\n';
		}
		ofs << "end\n";
		ofs << "# Labels: x y text (rgbcolor)\n";
		for (int iS=0; iS<eastend.size(); ++iS) {
			ofs
			<< (endstack[iS] + reclaim[iS]) / 2 << ' '
			<< (westend[iS] + eastend[iS])/2 << ' '
			<< (iS+1)
			<< '\n';
		}
		ofs << "end\n";
		for (int m=0; m<nr2; ++m) {
			ofs << " # Recl schedule: x y \n";
			map<int, int> mapReclTimes;
			for (int iS=0; iS<eastend.size(); ++iS)
				if ( m+1 == which[iS] )
					mapReclTimes[reclaim[iS]] = iS;
			if (mapReclTimes.empty())
				cout << "  !! Machine " << (m+1) << " not used. " << endl;
			for (map<int, int>::iterator ii=mapReclTimes.begin(); ii!=mapReclTimes.end(); ++ii)
				ofs
				<< ii->first << ' ' << westend[ii->second] << '\n'
				<< (finish[ii->second]) << ' ' << westend[ii->second] << '\n';
			ofs << "end\n";
		}

		// 2ND PAGE, LISTING THE INPUT DATA:
		ostringstream tss;
		tss
		<< "The input data: obj value " << obj
		<< " and:\\n\\n\\n";
		for (Parser::iterator ii=prs.begin(); ii!=prs.end(); ++ii)
			tss << ii->first << " = " << (*ii->second) << ";\\n";
		
		ofs
		<< "set title \"\\n\\n\\n\\n" << tss.str() << "\" font \"sans,12\"\n"
		<< "set yrange[-1:0]\n"
		<< "unset ytics; unset xtics; unset border\n"
		<< "plot x*x notitle\n\n";
		
		return sOutputFinal;
	}
};


int main(int argc, const char ** argv) {
	if (argc<2) {
		cerr << "Visualize pad / machine usage for a simple stockyard scheduling model.\n";
		cerr << "Give input (and gnuplot output) filename(s).\n";
		return -1;
	}
	Data data;
	ifstream ifs(argv[1]);
	data.Parse(ifs);                              // READ
	if (!ifs && !ifs.eof()) {
		perror(argv[1]);
		return -2;
	}
	data.Check();                                 // CHECK
	
	string sOutPlt = argv[1];
	if (argc>2)
		sOutPlt = argv[2];
	sOutPlt += ".plt";
	ofstream ofs(sOutPlt.c_str());
	string sOutFinal = data.OutputPlot(ofs, sOutPlt);    // PRINT GNUPLOT FILE
	ofs.close();
	if (!ofs) {
		perror(sOutPlt.c_str());
		return -3;
	}
	
	string cmd = "gnuplot ";
	cmd += sOutPlt;
	cout << "   Running '" << cmd << "'..." << endl;
	int res=system(cmd.c_str());                         // RUN GNUPLOT
	if (res) {
		cerr << "  gnuplot seems to fail, return status=" << res << endl;
		return -4;
	}
	return 0;
}