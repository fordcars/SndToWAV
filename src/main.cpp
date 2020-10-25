// Copyright 2020 Carl Hewett
//
// This file is part of SndToWAV.
//
// SndToWAV is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SndToWAV is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SndToWAV. If not, see <http://www.gnu.org/licenses/>.

#include "main.hpp"

#include "SndToWAV.hpp"

#include <cstddef> // For size_t
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

std::string gVersion = "v1.0";

using Big = long long int;

void printHelp()
{
    std::cout <<
        "********************************" << std::endl <<
        "**          SndToWAV          **" << std::endl <<
        "**        Version: " << gVersion << "      **" << std::endl <<
        "********************************" << std::endl <<
        std::endl <<
        "Converts a 'snd ' Apple Sound Manager sound effect into a RIFF .wav." << std::endl <<
        "Note: only supports format 1 'snd ' files containing a single sound sample." << std::endl <<
        std::endl <<
        "Usage: SndToWAV [-input INPUT_FILE -output OUTPUT_FILE]" << std::endl <<
        std::endl <<
        " --help, --h                 display help" << std::endl <<
        std::endl <<
        " -input                      snd file, should be a simple dump of a 'snd ' HFS+ resource" << std::endl <<
        " -output                     wav file to output" << std::endl;
}

int main(int argc, char **argv)
{
    // Terminal command, pointer to value to modify, textual type name.
    using argDefinitionTuple = std::tuple<std::string, void*, std::string>;
    using argDefinitionVector = std::vector<argDefinitionTuple>;

    // Modifiable with arguments
    std::string inputFile;
    std::string outputFile;

    // Wow! So easy!!!!!!!!!!!!!! :ooooo
    argDefinitionVector argDefinitions = {
                    argDefinitionTuple("--help", nullptr, "printHelp()"),
                    argDefinitionTuple("--h", nullptr, "printHelp()"),

                    argDefinitionTuple("-input", &inputFile, "std::string"),
                    argDefinitionTuple("-output", &outputFile, "std::string")
    };

    std::vector<std::string> args(argv, argv+argc);
    if(args.size() <= 1U) // This will never be < 1.
    {
        printHelp();
        return 0; // Quit
    }

    for(argDefinitionTuple argDefinition : argDefinitions)
    {
        std::string command = std::get<0>(argDefinition);
        void* associatedVariable = std::get<1>(argDefinition);
        std::string textualType = std::get<2>(argDefinition);

        // Find arg definition in args
        auto foundStringIt = std::find(args.begin(), args.end(), command);
        if(foundStringIt != args.end())
        {
            // Arg definition command found in given args!

            try
            {
                // Try to use the next argument as a parameter value.
                if(textualType == "std::string")
                    *static_cast<std::string*>(associatedVariable) = *(foundStringIt + 1); // Next arg

                else if(textualType == "int")
                    *static_cast<int*>(associatedVariable) = std::stoi(*(foundStringIt + 1));

                else if(textualType == "unsigned int")
                    *static_cast<unsigned int*>(associatedVariable) = std::stoul(*(foundStringIt + 1));

                else if(textualType == "std::size_t")
                    *static_cast<std::size_t*>(associatedVariable) = std::stoul(*(foundStringIt + 1));

                else if(textualType == "Big")
                    *static_cast<Big*>(associatedVariable) = std::stoll(*(foundStringIt + 1));

                else if(textualType == "float")
                    *static_cast<float*>(associatedVariable) = std::stof(*(foundStringIt + 1));
                else if(textualType == "printHelp()")
                {
                    printHelp();
                    return 0; // Quit
                }

            } catch(const std::invalid_argument& ia)
            {
                std::cerr << "Invalid value for '" + command + "'!"
                    << std::endl;
                return 1;
            }
        }
    }

    // Do errors:
    if(inputFile.empty())
    {
        std::cerr << "Error: input file not specified, you must specify it with -input" << std::endl;
        return 1;
    }

    if(outputFile.empty())
    {
        std::cerr << "Error: output file not specified, you must specify it with -input" << std::endl;
        return 1;
    }

    SndToWAV sndToWAV;
    sndToWAV.convert(inputFile, outputFile);
}
