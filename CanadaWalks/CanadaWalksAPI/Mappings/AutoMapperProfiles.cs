using AutoMapper;
using CanadaWalksAPI.Models.DTO;
using CanadaWalksAPI.Models.Domain;
using CanadaWalksAPI.Models.RTO;

namespace CanadaWalksAPI.Mappings
{
    public class AutoMapperProfiles : Profile
    {
        public AutoMapperProfiles()
        {
            CreateMap<RegionDTO, Region>().ReverseMap();
            CreateMap<AddRegionDTO, Region>().ReverseMap();
            CreateMap<UpdateRegionDTO, Region>().ReverseMap();
            CreateMap<AddWalkDTO, Walk>().ReverseMap();
            CreateMap<WalkDTO, Walk>().ReverseMap();
            CreateMap<UpdateWalkDTO, Walk>().ReverseMap();
            CreateMap<DifficultyDTO, Difficulty>().ReverseMap();
            CreateMap<UpdateWalkRTO, UpdateWalkDTO>().ReverseMap();
            CreateMap<UpdateRegionRTO, UpdateRegionDTO>().ReverseMap();
        }

    }
}
