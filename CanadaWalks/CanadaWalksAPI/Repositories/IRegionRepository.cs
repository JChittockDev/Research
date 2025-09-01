using CanadaWalksAPI.Models.Domain;
using CanadaWalksAPI.Models.RTO;

namespace CanadaWalksAPI.Repositories
{
    public interface IRegionRepository
    {
        Task<List<Region>> GetAllRegionsAsync();
        Task<Region?> GetRegionByIdAsync(Guid id);
        Task<Region> AddRegionAsync(Region region);
        Task<Region?> UpdateRegionAsync(Guid id, UpdateRegionRTO region);
        Task<bool?> DeleteRegionAsync(Guid id);
    }
}
