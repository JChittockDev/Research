using CanadaWalksAPI.Data;
using CanadaWalksAPI.Models.Domain;
using CanadaWalksAPI.Models.RTO;
using Microsoft.EntityFrameworkCore;

namespace CanadaWalksAPI.Repositories
{
    public class SQLRegionRepository : IRegionRepository
    {
        // This class is intended to implement the IRegionRepository interface
        // and provide methods for managing regions in a SQL database.
        private readonly CanadaWalksDbContext dbContext;

        // Constructor that accepts CanadaWalksDbContext
        public SQLRegionRepository(CanadaWalksDbContext dbContext)
        {
            this.dbContext = dbContext;
        }

        public async Task<Region> AddRegionAsync(Region region)
        {
            await dbContext.Regions.AddAsync(region);
            await dbContext.SaveChangesAsync();

            return region;
        }

        public async Task<bool?> DeleteRegionAsync(Guid id)
        {
            // Find the region by ID
            var region = await dbContext.Regions.FirstOrDefaultAsync(r => r.Id == id);

            // If the region is not found, return NotFound
            if (region == null)
            {
                return false;
            }

            // Remove the region from the database
            dbContext.Regions.Remove(region);
            await dbContext.SaveChangesAsync();

            return true;
        }

        public async Task<List<Region>> GetAllRegionsAsync()
        {
            return await dbContext.Regions.ToListAsync();
        }

        public async Task<Region?> GetRegionByIdAsync(Guid id)
        {
            return await dbContext.Regions.FirstOrDefaultAsync(r => r.Id == id);
        }

        public async Task<Region?> UpdateRegionAsync(Guid id, UpdateRegionRTO region)
        {
            // Find the existing region by ID
            var foundRegion = await dbContext.Regions.FirstOrDefaultAsync(r => r.Id == id);

            // If the region is not found, return NotFound
            if (foundRegion == null)
            {
                return null;
            }

            if (!string.IsNullOrEmpty(region.Code))
            {
                foundRegion.Code = region.Code;
            }

            if (!string.IsNullOrEmpty(region.Name))
            {
                foundRegion.Name = region.Name;
            }

            if (!string.IsNullOrEmpty(region.RegionImageUrl))
            {
                foundRegion.RegionImageUrl = region.RegionImageUrl;
            }

            await dbContext.SaveChangesAsync();

            return foundRegion;
        }
    }
}
