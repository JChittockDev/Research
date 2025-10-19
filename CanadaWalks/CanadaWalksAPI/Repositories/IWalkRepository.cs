using CanadaWalksAPI.Models.Domain;
using CanadaWalksAPI.Models.RTO;

namespace CanadaWalksAPI.Repositories
{
    public interface IWalkRepository
    {
        Task<List<Walk>> GetAllWalksAsync(
            string? filterOn = null, 
            string? filterQuery = null, 
            string? sortBy = null, 
            bool isAscending = true,
            int pageNumber = 1,
            int pageSize = 1000);
        Task<Walk?> GetWalkByIdAsync(Guid id);
        Task<Walk> AddWalkAsync(Walk walk);
        Task<Walk?> UpdateWalkAsync(Guid id, UpdateWalkRTO walk);
        Task<bool?> DeleteWalkAsync(Guid id);
    }
}
