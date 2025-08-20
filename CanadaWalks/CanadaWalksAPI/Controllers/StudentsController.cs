using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;

namespace CanadaWalksAPI.Controllers
{
    // https:localhost:7120/api/students
    [Route("api/[controller]")]
    [ApiController]
    public class StudentsController : ControllerBase
    {
        // GET: https:localhost:7120/api/students
        [HttpGet]
        public IActionResult GetAllStudents()
        {
            string[] students = new string[]
            {
                "John Doe",
                "Jane Smith",
                "Alice Johnson",
                "Bob Brown"
            };

            // Return the list of students as a 200 response
            return Ok(students);
        }
    }
}
