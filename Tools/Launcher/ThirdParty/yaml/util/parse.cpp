/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "yaml-cpp/yaml.h"
#include "yaml-cpp/eventhandler.h"
#include <fstream>
#include <iostream>
#include <vector>

struct Params {
	bool hasFile;
	std::string fileName;
};

Params ParseArgs(int argc, char **argv) {
	Params p;

	std::vector<std::string> args(argv + 1, argv + argc);
	
	return p;
}

class NullEventHandler: public YAML::EventHandler
{
public:
	virtual void OnDocumentStart(const YAML::Mark&) {}
	virtual void OnDocumentEnd() {}
	
	virtual void OnNull(const YAML::Mark&, YAML::anchor_t) {}
	virtual void OnAlias(const YAML::Mark&, YAML::anchor_t) {}
	virtual void OnScalar(const YAML::Mark&, const std::string&, YAML::anchor_t, const std::string&) {}
	
	virtual void OnSequenceStart(const YAML::Mark&, const std::string&, YAML::anchor_t) {}
	virtual void OnSequenceEnd() {}
	
	virtual void OnMapStart(const YAML::Mark&, const std::string&, YAML::anchor_t) {}
	virtual void OnMapEnd() {}
};

void parse(std::istream& input)
{
	try {
		YAML::Parser parser(input);
		YAML::Node doc;
		while(parser.GetNextDocument(doc)) {
			YAML::Emitter emitter;
			emitter << doc;
			std::cout << emitter.c_str() << "\n";
		}
	} catch(const YAML::Exception& e) {
		std::cerr << e.what() << "\n";
	}
}

int main(int argc, char **argv)
{
	Params p = ParseArgs(argc, argv);

	if(argc > 1) {
		std::ifstream fin;
		fin.open(argv[1]);
		parse(fin);
	} else {
		parse(std::cin);
	}

	return 0;
}
