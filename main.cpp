/*
 * main.cpp
 *
 *  Created on: 28.12.2016
 *      Author: andreas
 */

#include "simple_svg_1.0.0.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <will.pb.h>
#include <zip.h>

/** parse the length of the following protobuf stuff
 *
 * This function reads 128 bit (16 Byte) and calculates the length of the following protobuf section.
 * It sets the file pointer to the end of the parsed number.
 *
 * @param file handle to the file to read.
 *
 */
uint64_t getLength(zip_file_t *file)
{
    unsigned char data;
    data = 0x0;

    uint64_t len = 0;
    for (size_t i = 0; i < 16; i++)
    {
        zip_fread(file, (char *) &data, 1);
        len |= (data & 127) << (7 * i);
        // If the next-byte flag is set
        if (!(data & 128))
        {
            break;
        }
    }

    return len;
}

/** reads n chars from the file file, and returns a char array with the read data.
 *
 * @param file file handle to the file
 * @param len amount of chars to read.
 *
 */
unsigned char *getData(zip_file_t *file, int len)
{
    unsigned char *buff = (unsigned char *) calloc(sizeof(char), len);
    zip_fread(file, (char *) buff, len);
    return buff;
}

/** gernerates the path out of the protobuf steam part.
 *
 */
svg::Polyline getPath(unsigned char *data, uint len)
{
    WacomInkFormat::Path path;

    svg::Polyline polyline(svg::Fill(svg::Color::White), svg::Stroke(1, svg::Color::Black));

    svg::Point(0, 0);
    if (!path.ParseFromArray(data, len))
    {
        std::cerr << "Failed to parse will." << std::endl;
    }
    double dp = 0;
    dp = path.decimalprecision();

    if (path.points_size() > 0)
    {
        std::vector<int32_t> integer_values(path.points_size(), 0);

        integer_values[0] = path.points(0);
        integer_values[1] = path.points(1);
        for (int i = 2; i < path.points_size(); i += 2)
        {
            integer_values[i] = integer_values[i - 2] + path.points(i);
            integer_values[i + 1] = integer_values[i - 1] + path.points(i + 1);
        }

        for (int i = 0; i < path.points_size(); i += 2)
        {
            polyline << svg::Point(integer_values[i] / std::pow(10.0, dp), integer_values[i + 1] / std::pow(10.0, dp));
        }
    }

    return polyline;
}

void print_help(char *program_name)
{
    std::cerr << "Usage: " << std::string(program_name) << " -i input_filename [-o output_filename]\n";
}

/** Reads a protobuf file, and returns resulting svg line.
 *
 */
std::vector<svg::Polyline> read_file(const svg::Document &doc, zip_file_t *file)
{
    unsigned char *data;
    std::vector<svg::Polyline> lines;
    while (true)
    {
        auto len = getLength(file);
        if (len == 0)
        {
            break;
        }
        data = getData(file, len);
        lines.push_back(getPath(data, len));
        free(data);
    }
    return lines;
}

int main(int argc, char *argv[])
{
    int opt;

    std::string will_file_name;
    std::string svg_file_name;

    while ((opt = getopt(argc, argv, "i:o:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            will_file_name = std::string(optarg);
            break;
        case 'o':
            svg_file_name = std::string(optarg);
            break;
        default: /* '?' */
            print_help(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (will_file_name == "")
    {
        print_help(argv[0]);
        exit(EXIT_FAILURE);
    }

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    int error;
    zip_t *will_file = zip_open(will_file_name.c_str(), ZIP_RDONLY, &error);

    if (will_file == NULL)
    {
        std::cerr << "error opening will file. Libzip says: " << error << ":";
        switch (error)
        {
        case ZIP_ER_EXISTS:
            std::cerr << "ZIP_ER_EXISTS" << std::endl;
            break;
        case ZIP_ER_INCONS:
            std::cerr << "ZIP_ER_INCONS" << std::endl;
            break;
        case ZIP_ER_INVAL:
            std::cerr << "ZIP_ER_INVAL" << std::endl;
            break;
        case ZIP_ER_MEMORY:
            std::cerr << "ZIP_ER_MEMORY" << std::endl;
            break;
        case ZIP_ER_NOENT:
            std::cerr << "ZIP_ER_NOENT" << std::endl;
            break;
        case ZIP_ER_NOZIP:
            std::cerr << "ZIP_ER_NOZIP" << std::endl;
            break;
        case ZIP_ER_OPEN:
            std::cerr << "ZIP_ER_OPEN" << std::endl;
            break;
        case ZIP_ER_READ:
            std::cerr << "ZIP_ER_READ" << std::endl;
            break;
        case ZIP_ER_SEEK:
            std::cerr << "ZIP_ER_SEEK" << std::endl;
            break;
        default:
            std::cerr << std::endl;
            break;
        }
        exit(EXIT_FAILURE);
    }

    if (svg_file_name == "")
    {
        size_t file_name_length = will_file_name.find(".will");
        if (file_name_length == std::string::npos)
        {
            std::cerr << "not a .will file! Will append .svg" << std::endl;
            svg_file_name = will_file_name + ".svg";
        }
        else
        {
            svg_file_name = will_file_name.substr(0, file_name_length);
            svg_file_name += ".svg";
        }
    }

    zip_uint64_t i = 0;
    zip_stat_t file_stat;

    svg::Dimensions dimensions(592.0, 864.0);
    svg::Document doc(svg_file_name, svg::Layout(dimensions, svg::Layout::TopLeft));

    while (zip_stat_index(will_file, i, 0, &file_stat) == 0)
    {
        std::string file_name(file_stat.name);
        if (file_name.find(".protobuf") != std::string::npos && file_name.find("sections/media") != std::string::npos)
        {
            zip_file_t *file = zip_fopen_index(will_file, i, 0);
            if (file != NULL)
            {
                auto lines = read_file(doc, file);
                for (auto &line : lines)
                {
                    doc << line;
                }
            }
            else
            {
                std::cerr << "error opening " << file_name << std::endl;
            }
        }
        i++;
    }

    doc.save();
}
