using CanadaWalksAPI.Models.DTO;
using CanadaWalksAPI.Models.Domain;

namespace CanadaWalksAPI.Repositories
{
    public interface IRegionRepository
    {
        Task<List<Region>> GetAllRegionsAsync();
        Task<Region?> GetRegionByIdAsync(Guid id);
        Task<Region> AddRegionAsync(Region region);
        Task<Region?> UpdateRegionAsync(Guid id, Region region);
        Task<bool?> DeleteRegionAsync(Guid id);
    }
}
