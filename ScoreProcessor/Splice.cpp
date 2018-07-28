#include "stdafx.h"
#include "Splice.h"
#include "ScoreProcesses.h"
namespace ScoreProcessor {
	cil::CImg<unsigned char> splice_images(Splice::page const* imgs,size_t num,unsigned int padding)
	{
		unsigned int height=0;
		unsigned int width=0;
		unsigned int spectrum=0;
		for(size_t i=0;i<num;++i)
		{
			height+=imgs[i].img.height();
			if(imgs[i].img._width>width)
			{
				width=imgs[i].img._width;
			}
			if(imgs[i].img._spectrum>spectrum)
			{
				spectrum=imgs[i].img._spectrum;
			}
		}
		height+=padding*(num+1);
		cil::CImg<unsigned char> tog(width,height,1,spectrum);
		tog.fill(255);
		unsigned int ypos=padding;
		size_t const tsize=tog._width*tog._height;
		for(size_t i=0;i<num;++i)
		{
			auto const& current=imgs[i].img;
			size_t const csize=current._width*current._height;
			for(unsigned int y=0;y<current._height;++y)
			{
				unsigned int yabs=ypos+y-imgs[i].top;
				if(yabs<tog._height)
				{
					for(unsigned int x=1;x<=current._width;++x)
					{
						unsigned char* tpixel=&tog(tog._width,y);
						unsigned char const* cpixel=&current(current._width-x,y);
						auto eval_brightness=[](unsigned char const* pixel,size_t size,unsigned int spectrum)
						{
							switch(spectrum)
							{
								case 1:
									return (*pixel)*3U;
								case 3:
									return unsigned int(pixel[0])+pixel[size]+pixel[2*size];
								case 4:
									return (unsigned int(pixel[0])+pixel[size]+pixel[2*size])*pixel[3*size]/255U;
							}
						};
						unsigned int const tbrig=eval_brightness(tpixel,tsize,tog._spectrum);
						unsigned int const cbrig=eval_brightness(cpixel,csize,current._spectrum);
						if(cbrig<tbrig)
						{
							switch(tog._spectrum)
							{
								case 1:
									*tpixel=cbrig;
									break;
								case 3:
									switch(current._spectrum)
									{
										case 1:
											tpixel[0]=tpixel[tsize]=tpixel[2*tsize]=*cpixel;
											break;
										case 3:
											tpixel[0]=cpixel[0];
											tpixel[tsize]=cpixel[csize];
											tpixel[2*tsize]=cpixel[2*csize];
									}
									break;
								case 4:
									switch(current._spectrum)
									{
										case 1:
											tpixel[0]=tpixel[tsize]=tpixel[2*tsize]=*cpixel;
											break;
										case 3:
											tpixel[0]=cpixel[0];
											tpixel[tsize]=cpixel[csize];
											tpixel[2*tsize]=cpixel[2*csize];
											tpixel[3*tsize]=255;
											break;
										case 4:
											tpixel[0]=cpixel[0];
											tpixel[tsize]=cpixel[csize];
											tpixel[2*tsize]=cpixel[2*csize];
											tpixel[3*tsize]=cpixel[3*csize];
									}
							}
						}
					}
				}
			}
			ypos+=padding+current.height();
		}
		return tog;
	}

	struct spacing {
		unsigned int bottom_sg;
		unsigned int top_sg;
	};

	spacing find_spacing(std::vector<unsigned int> const& bottom_of_top,unsigned int size_top,std::vector<unsigned int> const& top_of_bottom)
	{
		auto b=bottom_of_top.rbegin();
		auto t=top_of_bottom.rbegin();
		auto end=b+std::min(bottom_of_top.size(),top_of_bottom.size());
		unsigned int min_spacing=std::numeric_limits<unsigned int>::max();
		spacing ret;
		for(;b!=end;++b,++t)
		{
			unsigned int cand=size_top-*b+*t;
			if(cand<min_spacing)
			{
				min_spacing=cand;
				ret.bottom_sg=*b;
				ret.top_sg=*t;
			}
		}
		return ret;
	}

	unsigned int splice_pages_parallel(
		std::vector<std::string> const& filenames,
		char const* output,
		unsigned int starting_index,
		unsigned int num_threads,
		Splice::standard_heuristics const& sh)
	{
		if(filenames.size()<2)
		{
			throw std::invalid_argument("Need multiple pages to splice");
		}
		std::vector<Splice::manager> managers(filenames.size());
		for(size_t i=0;i<filenames.size();++i)
		{
			managers[i].fname(filenames[i].c_str());
		}
		managers[0].load();
		unsigned int horiz_padding,min_pad,opt_pad,opt_height;
		std::array<unsigned int,2> bases{managers[0].img()._width,managers[0].img()._height};
		horiz_padding=sh.horiz_padding(bases);
		min_pad=sh.min_padding(bases);
		opt_pad=sh.optimal_padding(bases);
		opt_height=sh.optimal_height(bases);
		using Img=cil::CImg<unsigned char>;
		auto find_top=[bg=sh.background_color](Img const& img){
			if(img._spectrum<3)
			{
				return build_top_profile(img,bg);
			}
			else
			{
				return build_top_profile(img,ImageUtils::ColorRGB({bg,bg,bg}));
			}
		};
		auto find_bottom=[bg=sh.background_color](Img const& img){
			if(img._spectrum<3)
			{
				return build_bottom_profile(img,bg);
			}
			else
			{
				return build_bottom_profile(img,ImageUtils::ColorRGB({bg,bg,bg}));
			}
		};
		Splice::PageEval pe([=](Img const& img)
		{
			auto top=find_top(img);
			auto min=*std::min_element(top.begin(),top.end());
			return Splice::edge{min,min};
		},[=](Img const& t,Img const& b)
		{
			auto top=exlib::fattened_profile(find_bottom(t),horiz_padding,[](auto a,auto b)
			{
				return a>b;
			});
			auto bot=exlib::fattened_profile(find_top(b),horiz_padding,[](auto a,auto b)
			{
				return a<b;
			});
			auto top_max=*std::max_element(top.begin(),top.end());
			auto bot_max=*std::min_element(bot.begin(),bot.end());
			auto spacing=find_spacing(top,top_max,bot);
			return Splice::page_desc{{top_max,spacing.bottom_sg},{bot_max,spacing.top_sg}};
		},[=](Img const& img)
		{
			auto bottom=find_bottom(img);
			auto max=*std::max_element(bottom.begin(),bottom.end());
			return Splice::edge{max,max};
		});
		auto create_layout=[=](Splice::page_desc const* const items,size_t const n)
		{
			assert(n!=0);
			unsigned int total_height;
			if(n==1)
			{
				total_height=items[0].bottom.raw-items[0].top.raw;
			}
			else
			{
				total_height=items[0].bottom.kerned-items[0].top.raw;
				for(size_t i=1;i<n-1;++i)
				{
					total_height+=items[i].bottom.kerned-items[i].top.kerned;
				}
				total_height+=items[n-1].bottom.raw-items[n-1].top.kerned;
			}
			unsigned int minned=total_height+(n+1)*min_pad;
			if(minned>opt_height)
			{
				return Splice::page_layout{min_pad,minned};
			}
			else
			{
				return Splice::page_layout{unsigned int((opt_height-total_height)/(n+1)),opt_height};
			}
		};
		auto cost=[=](Splice::page_layout const p)
		{
			float numer;
			if(p.height>opt_height)
			{
				numer=sh.excess_weight*(p.height-opt_height);
			}
			else
			{
				numer=opt_height-p.height;
			}
			float height_cost=numer/opt_height;
			height_cost=height_cost*height_cost*height_cost;
			float padding_cost=sh.padding_weight*exlib::abs_dif(float(p.padding),opt_pad)/opt_pad;
			padding_cost=padding_cost*padding_cost*padding_cost;
			return height_cost+padding_cost;
		};
		return splice_pages_parallel(managers,output,starting_index,num_threads,pe,create_layout,cost);
	}
}