using CanadaWalksAPI.Models.DTO;
using CanadaWalksAPI.Repositories;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Identity;
using Microsoft.AspNetCore.Mvc;

namespace CanadaWalksAPI.Controllers
{
    [Route("api/[controller]")]
    [ApiController]
    public class AuthController : ControllerBase
    {
        private UserManager<IdentityUser> userManager;
        private ITokenRepository tokenRepository;
        public AuthController(UserManager<IdentityUser> userManager, ITokenRepository tokenRepository)
        {
            this.userManager = userManager;
            this.tokenRepository = tokenRepository;
        }
        // POST: https:localhost:7120/Api/Auth/Register
        [HttpPost]
        [Route("Register")]
        public async Task<IActionResult> Register([FromBody] RegisterRequestDTO registerRequestDTO)
        {
            var identityUser = new IdentityUser
            {
                UserName = registerRequestDTO.Username,
                Email = registerRequestDTO.Username
            };

            var identityResult = await userManager.CreateAsync(identityUser, registerRequestDTO.Password);

            if (!identityResult.Succeeded)
            {
                var errors = string.Join("; ", identityResult.Errors.Select(e => e.Description));
                return BadRequest($"Could not create user: {errors}");
            }

            if (registerRequestDTO.Roles != null && registerRequestDTO.Roles.Any())
            {
                var roleResult = await userManager.AddToRolesAsync(identityUser, registerRequestDTO.Roles);
                if (!roleResult.Succeeded)
                {
                    return BadRequest("Could not assign roles to user");
                }
            }

            return Ok("User Registered Successfully");
        }

        // POST: https:localhost:7120/Api/Auth/Login
        [HttpPost]
        [Route("Login")]
        public async Task<IActionResult> Login([FromBody] LoginRequestDTO loginRequestDTO)
        {
            var user = await userManager.FindByEmailAsync(loginRequestDTO.Username);

            if (user == null)
            {
                return BadRequest("Invalid Username");
            }

            var isValidPassword = await userManager.CheckPasswordAsync(user, loginRequestDTO.Password);

            if (!isValidPassword)
            {
                return BadRequest("Invalid Password");
            }

            var roles = await userManager.GetRolesAsync(user);

            if (!roles.Any() || roles == null)
            {
                return BadRequest("Failed to fetch user roles");
            }

            var token = tokenRepository.CreateJWTToken(user, roles.ToList());

            var response = new LoginResponseDTO
            {
                JwtToken = token
            };

            return Ok(response);
        }
    }
}
