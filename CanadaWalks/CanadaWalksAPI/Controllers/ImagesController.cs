using AutoMapper;
using CanadaWalksAPI.Models.Domain;
using CanadaWalksAPI.Models.DTO;
using CanadaWalksAPI.Repositories;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;

namespace CanadaWalksAPI.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class ImagesController : ControllerBase
    {
        private readonly IImageRepository imageRepository;
        private readonly IMapper mapper;

        public ImagesController(IImageRepository imageRepository, IMapper mapper)
        {
            this.imageRepository = imageRepository;
            this.mapper = mapper;
        }

        // POST: https:localhost:7120/Api/Images/Upload
        [HttpPost]
        [Route("Upload")]
        public async Task<IActionResult> Upload([FromForm] AddImageDTO requestDTO)
        {
            ValidateFileUpload(requestDTO);

            if (!ModelState.IsValid)
            {
                return BadRequest(ModelState);
            }

            var imageDomainModel = mapper.Map<Image>(requestDTO);

            var image = await imageRepository.UploadImageAsync(imageDomainModel);

            var imageResultDTO = mapper.Map<ImageDTO>(image);

            return Ok(imageResultDTO);
        }

        private void ValidateFileUpload(AddImageDTO requestDTO)
        {
            var allowedExtensions = new[] { ".jpg", ".jpeg", ".png", ".gif" };
            var maxFileSizeInBytes = 5 * 1024 * 1024; // 5 MB
            var fileExtension = Path.GetExtension(requestDTO.FileName).ToLower();
            if (!allowedExtensions.Contains(fileExtension))
            {
                ModelState.AddModelError("File", "Unsupported file type. Allowed types are: .jpg, .jpeg, .png, .gif");
            }
            if (requestDTO.File.Length > maxFileSizeInBytes)
            {
                ModelState.AddModelError("File", "File size exceeds the maximum limit of 5 MB.");
            }

        }

        // DELETE: https:localhost:7120/Api/Images/DeleteById/{id}
        [HttpDelete]
        [Route("DeleteById/{id:guid}")]
        public async Task<IActionResult> DeleteImageById([FromRoute] Guid id)
        {
            var result = await imageRepository.DeleteImageByIdAsync(id);
            if (result == null)
            {
                return StatusCode(StatusCodes.Status500InternalServerError, "An error occurred while deleting the image.");
            }
            if (result == false)
            {
                return NotFound("Image not found.");
            }
            return Ok("Image deleted successfully.");
        }

        // DELETE: https:localhost:7120/Api/Images/DeleteByName/{name}
        [HttpDelete]
        [Route("DeleteByName/{name}")]
        public async Task<IActionResult> DeleteImageByName([FromRoute] string name)
        {
            var result = await imageRepository.DeleteImageByNameAsync(name);
            if (result == null)
            {
                return StatusCode(StatusCodes.Status500InternalServerError, "An error occurred while deleting the image.");
            }
            if (result == false)
            {
                return NotFound("Image not found.");
            }
            return Ok("Image deleted successfully.");
        }

        // DELETE: https:localhost:7120/Api/Images/DeleteByPath
        [HttpDelete]
        [Route("DeleteByPath")]
        public async Task<IActionResult> DeleteImageByPath([FromQuery] string path)
        {
            var result = await imageRepository.DeleteImageByPathAsync(path);
            if (result == null)
            {
                return StatusCode(StatusCodes.Status500InternalServerError, "An error occurred while deleting the image.");
            }
            if (result == false)
            {
                return NotFound("Image not found.");
            }
            return Ok("Image deleted successfully.");
        }

        // GET: https:localhost:7120/Api/Images/GetById/{id}
        [HttpGet]
        [Route("GetById/{id:guid}")]
        public async Task<IActionResult> GetImageById([FromRoute] Guid id)
        {
            var image = await imageRepository.GetImageByIdAsync(id);
            if (image == null)
            {
                return NotFound("Image not found.");
            }

            var imageDTO = mapper.Map<ImageDTO>(image);

            return Ok(imageDTO);
        }

        // GET: https:localhost:7120/Api/Images/GetByName/{name}
        [HttpGet]
        [Route("GetByName/{name}")]
        public async Task<IActionResult> GetImageByName([FromRoute] string name)
        {
            var image = await imageRepository.GetImageByNameAsync(name);
            if (image == null)
            {
                return NotFound("Image not found.");
            }
            
            var imageDTO = mapper.Map<ImageDTO>(image);

            return Ok(imageDTO);
        }

        // GET: https:localhost:7120/Api/Images/GetByPath
        [HttpGet]
        [Route("GetByPath")]
        public async Task<IActionResult> GetImageByPath([FromQuery] string path)
        {
            var image = await imageRepository.GetImageByPathAsync(path);
            if (image == null)
            {
                return NotFound("Image not found.");
            }
            
            var imageDTO = mapper.Map<ImageDTO>(image);

            return Ok(imageDTO);
        }

    }
}
