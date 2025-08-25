using AutoMapper;
using CanadaWalksAPI.Models.DTO;
using CanadaWalksAPI.Models.Domain;

namespace CanadaWalksAPI.Mappings
{
    public class AutoMapperProfiles : Profile
    {
        public AutoMapperProfiles()
        {
            CreateMap<RegionDTO, Region>().ReverseMap();
            CreateMap<AddRegionDTO, Region>().ReverseMap();
            CreateMap<UpdateRegionDTO, Region>().ReverseMap();

        }

    }
}
