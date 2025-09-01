using Microsoft.AspNetCore.Mvc;
using CanadaWalksAPI.Models.DTO;
using CanadaWalksAPI.Models.RTO;
using CanadaWalksAPI.Models.Domain;

using Microsoft.EntityFrameworkCore;
using CanadaWalksAPI.Repositories;
using AutoMapper;

namespace CanadaWalksAPI.Controllers
{
    // https:localhost:7120/api/regions
    [Route("api/[controller]")]
    [ApiController]
    public class RegionsController : ControllerBase
    {
        private readonly IRegionRepository regionRepository;
        private readonly IMapper mapper;

        // Constructor that accepts CanadaWalksDbContext
        public RegionsController(IRegionRepository regionRepository, IMapper mapper)
        {
            this.regionRepository = regionRepository;
            this.mapper = mapper;
        }

        // async declares the method as asynchronous, allowing it to run without blocking the thread
        // Task <IActionResult> is the return type, indicating that the method will return an IActionResult wrapped in a Task
        // GET: https:localhost:7120/api/regions
        [HttpGet]
        public async Task<IActionResult> GetAllRegions()
        {
            // Retrieve all regions from the database
            // using ToListAsync() to asynchronously fetch the list of regions, this ensures the thread is not blocked while waiting for the database operation to complete
            var regions = await regionRepository.GetAllRegionsAsync();

            // Because fetching the list is async, we need to await the result
            // This loop does not need to be async, as we are simply mapping the domain objects to DTOs
            // Map region domain objects to DTOs
            //var regionsDTO = new List<RegionDTO>();
            //foreach (var region in regions)
            //{
            //    regionsDTO.Add(new RegionDTO
            //    {
            //        Id = region.Id,
            //        Code = region.Code,
            //        Name = region.Name,
            //        RegionImageUrl = region.RegionImageUrl // This can be null
            //    });
            //}

            // Use AutoMapper to map the list of regions to a list of RegionDTOs
            var regionsDTO = mapper.Map<List<RegionDTO>>(regions);

            // Return the list of regions as a 200 response
            return Ok(regionsDTO);
        }

        // GET: https:localhost:7120/api/regions/{id}
        [HttpGet]
        [Route("{id:Guid}")] // Route parameter for Guid
        public async Task<IActionResult> GetRegionById([FromRoute] Guid id)
        {
            // Find the region by ID
            var region = await regionRepository.GetRegionByIdAsync(id);

            // If the region is found, map it to a DTO
            if (region != null)
            {
                //var regionDTO = new RegionDTO
                //{
                //    Id = region.Id,
                //    Code = region.Code,
                //    Name = region.Name,
                //    RegionImageUrl = region.RegionImageUrl // This can be null
                //};

                // Use AutoMapper to map the region to a RegionDTO
                var regionDTO = mapper.Map<RegionDTO>(region);

                // Return the found region as a 200 response
                return Ok(regionDTO);
            }

            return NotFound();

        }

        // POST: https:localhost:7120/api/regions
        [HttpPost]
        public async Task<IActionResult> CreateRegion([FromBody] AddRegionDTO addRegionDTO)
        {
            // Validate the incoming DTO
            if (addRegionDTO == null || string.IsNullOrEmpty(addRegionDTO.Code) || string.IsNullOrEmpty(addRegionDTO.Name))
            {
                return BadRequest("Invalid region data.");
            }

            //// Map DTO to domain model
            //var region = new Region
            //{
            //    Id = Guid.NewGuid(), // Generate a new unique ID
            //    Code = addRegionDTO.Code,
            //    Name = addRegionDTO.Name,
            //    RegionImageUrl = addRegionDTO.RegionImageUrl // This can be null
            //};

            // Use AutoMapper to map the AddRegionDTO to a Region domain model
            var region = mapper.Map<Region>(addRegionDTO);

            await regionRepository.AddRegionAsync(region);

            // Return the created region as a 201 response with the location header
            return CreatedAtAction(nameof(GetRegionById), new { id = region.Id }, region);
        }

        // PUT: https:localhost:7120/api/regions
        [HttpPut]
        [Route("{id:Guid}")] // Route parameter for Guid
        public async Task<IActionResult> UpdateRegion([FromRoute] Guid id, [FromBody] UpdateRegionDTO updateRegionDTO)
        {
            // Validate the incoming DTO
            if (updateRegionDTO == null)
            {
                return BadRequest("Invalid region data.");
            }

            //// Convert UpdateRegionDTO to Region domain model
            //var domainRegion = new Region
            //{
            //    Id = id, // Use the ID from the route
            //    Code = updateRegionDTO.Code,
            //    Name = updateRegionDTO.Name,
            //    RegionImageUrl = updateRegionDTO.RegionImageUrl // This can be null
            //};

            // Use AutoMapper to map the UpdateRegionDTO to a Region domain model
            var domainRegion = mapper.Map<UpdateRegionRTO>(updateRegionDTO);

            // Update the region using the repository
            Region region = await regionRepository.UpdateRegionAsync(id, domainRegion);

            if (region == null)
            {
                // If the region is not found, return NotFound
                return NotFound();
            }

            //// Return the updated region as a 200 response
            //var updatedRegionDTO = new RegionDTO
            //{
            //    Id = region.Id,
            //    Code = region.Code,
            //    Name = region.Name,
            //    RegionImageUrl = region.RegionImageUrl // This can be null
            //};

            // Use AutoMapper to map the updated region to a RegionDTO
            var updatedRegionDTO = mapper.Map<RegionDTO>(region);

            return Ok(updatedRegionDTO);
        }

        // DELETE: https:localhost:7120/api/regions/{id}
        [HttpDelete]
        [Route("{id:Guid}")] // Route parameter for Guid
        public async Task<IActionResult> DeleteRegion([FromRoute] Guid id)
        {
            // Attempt to delete the region by ID
            var isDeleted = await regionRepository.DeleteRegionAsync(id);
            
            if (isDeleted == false)
            {
                // If the region is not found, return NotFound
                return NotFound();
            }

            return NoContent();
        }
    }

}
