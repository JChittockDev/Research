using System;
using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

#pragma warning disable CA1814 // Prefer jagged arrays over multidimensional

namespace CanadaWalksAPI.Migrations
{
    /// <inheritdoc />
    public partial class Seed1 : Migration
    {
        /// <inheritdoc />
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.InsertData(
                table: "Regions",
                columns: new[] { "Id", "Code", "Name", "RegionImageUrl" },
                values: new object[,]
                {
                    { new Guid("a4b5c6d7-e8f9-0a1b-2c3d-4e5f6a7b8c9d"), "QC", "Quebec", "https://example.com/images/qc.jpg" },
                    { new Guid("d1e2f3a4-b5c6-7d8e-9f0a-1b2c3d4e5f60"), "BC", "British Columbia", "https://example.com/images/bc.jpg" },
                    { new Guid("e2f3a4b5-c6d7-8e9f-0a1b-2c3d4e5f6a7b"), "AB", "Alberta", "https://example.com/images/ab.jpg" },
                    { new Guid("f3a4b5c6-d7e8-9f0a-1b2c-3d4e5f6a7b8c"), "ON", "Ontario", "https://example.com/images/on.jpg" }
                });
        }

        /// <inheritdoc />
        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DeleteData(
                table: "Regions",
                keyColumn: "Id",
                keyValue: new Guid("a4b5c6d7-e8f9-0a1b-2c3d-4e5f6a7b8c9d"));

            migrationBuilder.DeleteData(
                table: "Regions",
                keyColumn: "Id",
                keyValue: new Guid("d1e2f3a4-b5c6-7d8e-9f0a-1b2c3d4e5f60"));

            migrationBuilder.DeleteData(
                table: "Regions",
                keyColumn: "Id",
                keyValue: new Guid("e2f3a4b5-c6d7-8e9f-0a1b-2c3d4e5f6a7b"));

            migrationBuilder.DeleteData(
                table: "Regions",
                keyColumn: "Id",
                keyValue: new Guid("f3a4b5c6-d7e8-9f0a-1b2c-3d4e5f6a7b8c"));
        }
    }
}
