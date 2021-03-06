// Trainer.cpp : Defines the entry point for the console application.
// This application will be the one used to train the machine learning algorithms.

#include "stdafx.h"
#include <iostream>
#include "../NeuralNetwork/neural_scaler.h"
#include <cstdlib>
#include <chrono>
#include <string>
#include "../ScoreProcessor/lib/threadpool/thread_pool.h"

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
void train(scaler& scl,Img const& answer,Img const& input,float learning_rate,size_t batch_size)
{
	auto& net=scl.net();
	auto const input_dim=scl.input_dim();
	auto const input_area=input_dim*input_dim;
	auto const output_dim=scl.output_dim();
	auto const output_area=output_dim*output_dim;
	auto const padding=scl.padding();
	auto const scale=scl.scale_factor();
	if(input_dim>input._height||input_dim>input._width)
	{
		throw std::invalid_argument("Answer image must be larger than input dim");
	}
	std::unique_ptr<float[]> data(new float[batch_size*(output_area+input_area)]);
	auto const input_data=data.get();
	auto const output_data=data.get()+batch_size*input_area;

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
	using point=std::pair<unsigned int,unsigned int>;
	std::vector<point> points;
	auto point_count=x_limit*y_limit;
	auto const r=point_count%batch_size;
	if(r!=0)
	{
		point_count+=(batch_size-r);
	}
	points.reserve(point_count);
	for(unsigned int x=0;x<x_limit;++x)
	{
		for(unsigned int y=0;y<y_limit;++y)
		{
			points.push_back(std::make_pair(x,y));
		}
	}
	std::random_device rd;
	std::mt19937_64 mt(rd());
	std::uniform_int_distribution<unsigned int> pdis;
	while(points.size()<point_count)
	{
		points.push_back(std::make_pair(pdis(mt)%x_limit,pdis(mt)%y_limit));
	}
	std::shuffle(points.begin(),points.end(),mt);
	struct info {
		neural_net::results result;
		neural_net::delta_t deltas;
		std::vector<neural_net::layer> gradients;
	};
	std::vector<info> results;
	results.reserve(batch_size);
	using Net=std::remove_reference_t<decltype(net)>;
	for(size_t i=0;i<batch_size;++i)
	{
		results.emplace_back(
			info{
				neural_net::results(net.layers()),
				neural_net::delta_t(net.layers()),
				neural_net::construct_layers(net.layers())
			});
	}
	auto const adjustment=learning_rate/batch_size;
	exlib::thread_pool_a<Net const&,Img const&,Img const&,unsigned int,unsigned int,unsigned int,unsigned int> 
		pool(std::max<size_t>(2,std::thread::hardware_concurrency()),
			net,answer,input,scale,padding,input_dim,output_dim);
	struct Task {
	private:
		point p;
		float* input;
		float* output;
		info& results;
	public:
		Task(point p,float* i,float* o,info& r):p(p),input(i),output(o),results(r)
		{}
		void execute(Net const& net,Img const& answer,Img const& orig,unsigned int scale,unsigned int padding,unsigned int input_dim,unsigned int output_dim) const
		{
			//auto const msg=std::to_string(p.first).append(" ").append(std::to_string(p.second));
			//std::cout<<msg+"\n";
			fill_in(this->input,orig,p.first,p.second,input_dim);
			fill_in(output,answer,scale*p.first+padding,scale*p.second+padding,output_dim);
			net.feed_forward_store(results.result,this->input);
			net.calculate_deltas(results.deltas,results.result,output);
			net.calculate_grads(results.gradients.data(),results.deltas,results.result);
			//std::cout<<msg+" done\n";
		}
	};
	for(size_t i=0;i<points.size();i+=batch_size)
	{
		//std::cout<<"Batch "<<i/batch_size<<'\n';
		for(size_t j=0;j<batch_size;++j)
		{
			pool.push_back([task=Task{points[i+j],input_data+j*input_area,output_data+j*output_area,results[j]}]
			(Net const& net,Img const& answer,Img const& orig,unsigned int scale,unsigned int padding,unsigned int input_dim,unsigned int output_dim){
				task.execute(net,answer,orig,scale,padding,input_dim,output_dim);
			});
		}
		pool.wait();
		//std::cout<<"Updating\n";
		for(size_t j=0;j<batch_size;++j)
		{
			net.update_weights(results[j].gradients.data(),adjustment);
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
		float learning_rate=0.01;
		size_t batch_size=10;
		if(argc>5)
		{
			learning_rate=std::min(learning_rate,std::strtof(argv[5],nullptr));
			if(argc>6)
			{
				batch_size=std::min(batch_size,static_cast<size_t>(std::strtoull(argv[6],nullptr,10)));
			}
		}
		train(my_scaler,answer,input,learning_rate,batch_size);
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

