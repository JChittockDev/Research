using CanadaWalksAPI.Models.Domain;
using CanadaWalksAPI.Models.RTO;

namespace CanadaWalksAPI.Repositories
{
    public interface IWalkRepository
    {
        Task<List<Walk>> GetAllWalksAsync();
        Task<Walk?> GetWalkByIdAsync(Guid id);
        Task<Walk> AddWalkAsync(Walk walk);
        Task<Walk?> UpdateWalkAsync(Guid id, UpdateWalkRTO walk);
        Task<bool?> DeleteWalkAsync(Guid id);
    }
}
