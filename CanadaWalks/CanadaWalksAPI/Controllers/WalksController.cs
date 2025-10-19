using AutoMapper;
using CanadaWalksAPI.CustomActionFilters;
using CanadaWalksAPI.Models.Domain;
using CanadaWalksAPI.Models.DTO;
using CanadaWalksAPI.Models.RTO;
using CanadaWalksAPI.Repositories;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;

namespace CanadaWalksAPI.Controllers
{
    // https:localhost:7120/api/walks
    [Route("api/[controller]")]
    [ApiController]
    public class WalksController : ControllerBase
    {
        private readonly IWalkRepository walkRepository;
        private readonly IMapper mapper;

        // Constructor that accepts CanadaWalksDbContext
        public WalksController(IWalkRepository walkRepository, IMapper mapper)
        {
            this.walkRepository = walkRepository;
            this.mapper = mapper;
        }

        // GET: https:localhost:7120/api/walks?fileterOn=Name&filterQuery=Track&sortBy=Name&IsAscending=true&pageNumber=1&pageSize=10
        // Query parameters can be used for filtering, sorting, and pagination
        [HttpGet]
        public async Task<IActionResult> GetAllWalks(
            [FromQuery] string? filterOn, [FromQuery] string? filterQuery, 
            [FromQuery] string? sortBy, [FromQuery] bool? isAscending,
            [FromQuery] int pageNumber = 1, [FromQuery] int pageSize = 1000)
        {
            var walks = await walkRepository.GetAllWalksAsync(filterOn, filterQuery, sortBy, isAscending ?? true, pageNumber, pageSize);
            var walksDTO = mapper.Map<List<WalkDTO>>(walks);

            // Return the list of walks as a 200 response
            return Ok(walksDTO);
        }

        // GET: https:localhost:7120/api/walks/{id}
        [HttpGet]
        [Route("{id:Guid}")] // Route parameter for Guid
        public async Task<IActionResult> GetWalkById([FromRoute] Guid id)
        {
            // Find the walk by ID
            var walk = await walkRepository.GetWalkByIdAsync(id);

            // If the walk is found, map it to a DTO
            if (walk != null)
            {
                // Use AutoMapper to map the walk to a WalkDTO
                var walkDTO = mapper.Map<WalkDTO>(walk);

                // Return the found walk as a 200 response
                return Ok(walkDTO);
            }

            return NotFound();

        }

        // POST: https:localhost:7120/api/walks
        [HttpPost]
        [ValidateModel]
        public async Task<IActionResult> CreateWalk([FromBody] AddWalkDTO addWalkDTO)
        {
            // Validate the incoming DTO
            if (addWalkDTO == null || string.IsNullOrEmpty(addWalkDTO.Name) || string.IsNullOrEmpty(addWalkDTO.Description))
            {
                return BadRequest("Invalid walk data - NULL");
            }

            // Use AutoMapper to map the AddWalkDTO to a Walk domain model
            var walk = mapper.Map<Walk>(addWalkDTO);

            await walkRepository.AddWalkAsync(walk);

            // Return the created walk as a 201 response with the location header
            return CreatedAtAction(nameof(GetWalkById), new { id = walk.Id }, addWalkDTO);
        }

        // PUT: https:localhost:7120/api/walks
        [HttpPut]
        [ValidateModel]
        [Route("{id:Guid}")] // Route parameter for Guid
        public async Task<IActionResult> UpdateWalk([FromRoute] Guid id, [FromBody] UpdateWalkDTO updateWalkDTO)
        {
            // Validate the incoming DTO
            if (updateWalkDTO == null)
            {
                return BadRequest("Invalid walk data - NULL");
            }


            // Use AutoMapper to map the UpdateWalkDTO to a Walk domain model
            var domainWalk = mapper.Map<UpdateWalkRTO>(updateWalkDTO);

            // Update the walk using the repository
            Walk walk = await walkRepository.UpdateWalkAsync(id, domainWalk);

            if (walk == null)
            {
                // If the walk is not found, return NotFound
                return NotFound();
            }

            // Use AutoMapper to map the updated walk to a WalkDTO
            var updatedWalkDTO = mapper.Map<WalkDTO>(walk);

            return Ok(updatedWalkDTO);
        }

        // DELETE: https:localhost:7120/api/walks/{id}
        [HttpDelete]
        [Route("{id:Guid}")] // Route parameter for Guid
        public async Task<IActionResult> DeleteWalk([FromRoute] Guid id)
        {
            // Attempt to delete the walk by ID
            var isDeleted = await walkRepository.DeleteWalkAsync(id);

            if (isDeleted == false)
            {
                // If the walk is not found, return NotFound
                return NotFound();
            }

            return NoContent();
        }
    }
}
