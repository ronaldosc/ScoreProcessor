// Trainer.cpp : Defines the entry point for the console application.
// This application will be the one used to train the machine learning algorithms.

#include "stdafx.h"
#include <iostream>
#include "../NeuralNetwork/neural_scaler.h"
#include <cstdlib>
#include <chrono>
#include <string>

using Img=cil::CImg<unsigned char>;
using net=neural_net::net<>;
using scaler=ScoreProcessor::neural_scaler;

void fill_in(float* const out,Img const& img,unsigned int const x,unsigned int const y,unsigned int const dim)
{
	auto idata=img._data+y*img._width+x;
	for(unsigned int yc=0;yc<dim;++yc)
	{
		auto const img_row=idata+yc*img._width;
		auto const data=out+yc*dim;
		for(unsigned int xc=0;xc<dim;++xc)
		{
			data[xc]=img_row[xc]/255.0f;
		}
	}
}
bool all_white(float const* d,size_t l)
{
	for(size_t i=0;i<l;++i)
	{
		if(d[i]!=1)
		{
			return false;
		}
	}
	return true;
}
void train(scaler& scl,Img const& answer,Img const& input,unsigned int scale)
{
	auto& net=scl.net();
	auto const input_dim=scl.input_dim();
	auto const output_dim=scl.output_dim();
	auto const padding=scl.padding();
	if(input_dim>answer._height||input_dim>answer._width)
	{
		throw std::invalid_argument("Answer image must be larger than input dim");
	}
	std::unique_ptr<float[]> ninput(new float[net.layers().front().neuron_count()]);
	std::unique_ptr<float[]> nanswer(new float[net.layers().back().neuron_count()]);
#ifndef NDEBUG
	auto view=[](auto data,auto dim)
	{
		cil::CImg<float> img;
		img._data=data;
		img._depth=1;
		img._spectrum=1;
		img._height=dim;
		img._width=dim;
		img.display();
	};
#endif
	auto const x_limit=input._width-input_dim;
	auto const y_limit=input._height-input_dim;
	for(unsigned int x=0;x<x_limit;++x)
	{
		for(unsigned int y=0;y<y_limit;++y)
		{
			fill_in(ninput.get(),input,x,y,input_dim);
			if(all_white(ninput.get(),input_dim*input_dim))
			{
				continue;
			}
			fill_in(nanswer.get(),answer,(x+padding)*scale,(y+padding)*scale,output_dim);
			net.train(ninput.get(),nanswer.get(),1,1E-4f);
		}
	}
}
bool has_comma(char const* str)
{
	while(*str)
	{
		if(*str==',') return true;
		++str;
	}
	return false;
}
std::vector<unsigned int> parse_csv(char const* str)
{
	std::vector<unsigned int> ret;
	auto& err=errno;
	while(1)
	{
		char* end;
		err=0;
		auto val=std::strtoul(str,&end,10);
		if(err)
		{
			throw std::invalid_argument("Invalid values");
		}
		ret.push_back(val);
		if(*end=='\0')
		{
			ret.front()*=ret.front();
			ret.back()*=ret.back();
			return ret;
		}
		if(*end!=',')
		{
			throw std::invalid_argument("Invalid input");
		}
		str=end+1;
	}
}
int main(int argc,char** argv)
{
	if(argc<3)
	{
		std::cout<<
			__DATE__ " " __TIME__ "\n"
			"First arg is answer file.\n"
			"Second arg is input file.\n"
			"Third arg is CSV of layer heights (first and last values are squared) OR\n"
			"Third arg is name of file to base training on\n"
			"Fourth arg is name to save training file as, def %timestamp%.ssn\n";
		return 0;
	}
	try
	{
		cil::CImg<unsigned char> answer(argv[1]),input(argv[2]);
		unsigned int scale_factor;
		if(answer._height<=input._width||answer._height%input._height||answer._width%input._width||(scale_factor=answer._height/input._height)!=answer._width/input._width)
		{
			std::cout<<"Answer dimensions must be an (Natural + 2) multiple of the input dimensions\n";
			return 0;
		}
		auto my_scaler=has_comma(argv[3])?
			scaler(scale_factor,std::move(net(parse_csv(argv[3])).randomize())):
			scaler(argv[3]);
		if(my_scaler.scale_factor()!=scale_factor)
		{
			std::cout<<"Image scale does not fit scale factor of given file.\n";
			return 0;
		}
		train(my_scaler,answer,input,scale_factor);
		if(argc>4)
		{
			my_scaler.save(argv[4]);
		}
		else
		{
			auto name=std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
			name.append(".ssn");
			my_scaler.save(name.c_str());
		}
	}
	catch(std::exception const& err)
	{
		std::cout<<err.what()<<'\n';
		return 1;
	}
	std::cout<<"Done\n";
	return 0;
}

