using Microsoft.EntityFrameworkCore.Migrations;
using Npgsql.EntityFrameworkCore.PostgreSQL.Metadata;

namespace CSharpParser.Migrations
{
    public partial class Initial : Migration
    {
        protected override void Up(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.CreateTable(
                name: "CsharpAstNodes",
                columns: table => new
                {
                    Id = table.Column<decimal>(type: "numeric(20,0)", nullable: false),
                    AstValue = table.Column<string>(type: "text", nullable: true),
                    AstSymbolType = table.Column<int>(type: "integer", nullable: false),
                    AstType = table.Column<int>(type: "integer", nullable: false),
                    Accessibility = table.Column<int>(type: "integer", nullable: false),
                    Location_range_start_line = table.Column<long>(type: "bigint", nullable: false),
                    Location_range_start_column = table.Column<long>(type: "bigint", nullable: false),
                    Location_range_end_line = table.Column<long>(type: "bigint", nullable: false),
                    Location_range_end_column = table.Column<long>(type: "bigint", nullable: false),
                    Path = table.Column<string>(type: "text", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    RawKind = table.Column<int>(type: "integer", nullable: false)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpAstNodes", x => x.Id);
                });

            migrationBuilder.CreateTable(
                name: "CsharpEtcEntitys",
                columns: table => new
                {
                    Id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    EtcEntityType = table.Column<int>(type: "integer", nullable: false),
                    DeclaratorNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: false),
                    AstNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    ParentNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    Name = table.Column<string>(type: "text", nullable: true),
                    QualifiedName = table.Column<string>(type: "text", nullable: true),
                    DocumentationCommentXML = table.Column<string>(type: "text", nullable: true),
                    TypeHash = table.Column<long>(type: "bigint", nullable: false),
                    QualifiedType = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpEtcEntitys", x => x.Id);
                    table.ForeignKey(
                        name: "FK_CsharpEtcEntitys_CsharpAstNodes_AstNodeId",
                        column: x => x.AstNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpEtcEntitys_CsharpAstNodes_ParentNodeId",
                        column: x => x.ParentNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                });

            migrationBuilder.CreateTable(
                name: "CsharpMethods",
                columns: table => new
                {
                    Id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    MethodType = table.Column<int>(type: "integer", nullable: false),
                    AstNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    ParentNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    Name = table.Column<string>(type: "text", nullable: true),
                    QualifiedName = table.Column<string>(type: "text", nullable: true),
                    DocumentationCommentXML = table.Column<string>(type: "text", nullable: true),
                    TypeHash = table.Column<long>(type: "bigint", nullable: false),
                    QualifiedType = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpMethods", x => x.Id);
                    table.ForeignKey(
                        name: "FK_CsharpMethods_CsharpAstNodes_AstNodeId",
                        column: x => x.AstNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpMethods_CsharpAstNodes_ParentNodeId",
                        column: x => x.ParentNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                });

            migrationBuilder.CreateTable(
                name: "CsharpNamespaces",
                columns: table => new
                {
                    Id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    AstNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    ParentNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    Name = table.Column<string>(type: "text", nullable: true),
                    QualifiedName = table.Column<string>(type: "text", nullable: true),
                    DocumentationCommentXML = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpNamespaces", x => x.Id);
                    table.ForeignKey(
                        name: "FK_CsharpNamespaces_CsharpAstNodes_AstNodeId",
                        column: x => x.AstNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpNamespaces_CsharpAstNodes_ParentNodeId",
                        column: x => x.ParentNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                });

            migrationBuilder.CreateTable(
                name: "CsharpVariables",
                columns: table => new
                {
                    Id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    VariableType = table.Column<int>(type: "integer", nullable: false),
                    AstNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    ParentNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    Name = table.Column<string>(type: "text", nullable: true),
                    QualifiedName = table.Column<string>(type: "text", nullable: true),
                    DocumentationCommentXML = table.Column<string>(type: "text", nullable: true),
                    TypeHash = table.Column<long>(type: "bigint", nullable: false),
                    QualifiedType = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpVariables", x => x.Id);
                    table.ForeignKey(
                        name: "FK_CsharpVariables_CsharpAstNodes_AstNodeId",
                        column: x => x.AstNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpVariables_CsharpAstNodes_ParentNodeId",
                        column: x => x.ParentNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                });

            migrationBuilder.CreateTable(
                name: "CsharpClasses",
                columns: table => new
                {
                    Id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    ClassType = table.Column<int>(type: "integer", nullable: false),
                    CsharpNamespaceId = table.Column<long>(type: "bigint", nullable: true),
                    AstNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    ParentNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    Name = table.Column<string>(type: "text", nullable: true),
                    QualifiedName = table.Column<string>(type: "text", nullable: true),
                    DocumentationCommentXML = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpClasses", x => x.Id);
                    table.ForeignKey(
                        name: "FK_CsharpClasses_CsharpAstNodes_AstNodeId",
                        column: x => x.AstNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpClasses_CsharpAstNodes_ParentNodeId",
                        column: x => x.ParentNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpClasses_CsharpNamespaces_CsharpNamespaceId",
                        column: x => x.CsharpNamespaceId,
                        principalTable: "CsharpNamespaces",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                });

            migrationBuilder.CreateTable(
                name: "CsharpEnums",
                columns: table => new
                {
                    Id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    CsharpNamespaceId = table.Column<long>(type: "bigint", nullable: true),
                    AstNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    ParentNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    Name = table.Column<string>(type: "text", nullable: true),
                    QualifiedName = table.Column<string>(type: "text", nullable: true),
                    DocumentationCommentXML = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpEnums", x => x.Id);
                    table.ForeignKey(
                        name: "FK_CsharpEnums_CsharpAstNodes_AstNodeId",
                        column: x => x.AstNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpEnums_CsharpAstNodes_ParentNodeId",
                        column: x => x.ParentNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpEnums_CsharpNamespaces_CsharpNamespaceId",
                        column: x => x.CsharpNamespaceId,
                        principalTable: "CsharpNamespaces",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                });

            migrationBuilder.CreateTable(
                name: "CsharpStructs",
                columns: table => new
                {
                    Id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    CsharpNamespaceId = table.Column<long>(type: "bigint", nullable: true),
                    AstNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    ParentNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    Name = table.Column<string>(type: "text", nullable: true),
                    QualifiedName = table.Column<string>(type: "text", nullable: true),
                    DocumentationCommentXML = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpStructs", x => x.Id);
                    table.ForeignKey(
                        name: "FK_CsharpStructs_CsharpAstNodes_AstNodeId",
                        column: x => x.AstNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpStructs_CsharpAstNodes_ParentNodeId",
                        column: x => x.ParentNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpStructs_CsharpNamespaces_CsharpNamespaceId",
                        column: x => x.CsharpNamespaceId,
                        principalTable: "CsharpNamespaces",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                });

            migrationBuilder.CreateTable(
                name: "CsharpEnumMembers",
                columns: table => new
                {
                    Id = table.Column<long>(type: "bigint", nullable: false)
                        .Annotation("Npgsql:ValueGenerationStrategy", NpgsqlValueGenerationStrategy.IdentityByDefaultColumn),
                    EqualsValue = table.Column<int>(type: "integer", nullable: false),
                    CsharpEnumId = table.Column<long>(type: "bigint", nullable: true),
                    AstNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    ParentNodeId = table.Column<decimal>(type: "numeric(20,0)", nullable: true),
                    EntityHash = table.Column<long>(type: "bigint", nullable: false),
                    Name = table.Column<string>(type: "text", nullable: true),
                    QualifiedName = table.Column<string>(type: "text", nullable: true),
                    DocumentationCommentXML = table.Column<string>(type: "text", nullable: true)
                },
                constraints: table =>
                {
                    table.PrimaryKey("PK_CsharpEnumMembers", x => x.Id);
                    table.ForeignKey(
                        name: "FK_CsharpEnumMembers_CsharpAstNodes_AstNodeId",
                        column: x => x.AstNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpEnumMembers_CsharpAstNodes_ParentNodeId",
                        column: x => x.ParentNodeId,
                        principalTable: "CsharpAstNodes",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                    table.ForeignKey(
                        name: "FK_CsharpEnumMembers_CsharpEnums_CsharpEnumId",
                        column: x => x.CsharpEnumId,
                        principalTable: "CsharpEnums",
                        principalColumn: "Id",
                        onDelete: ReferentialAction.Restrict);
                });

            migrationBuilder.CreateIndex(
                name: "IX_CsharpClasses_AstNodeId",
                table: "CsharpClasses",
                column: "AstNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpClasses_CsharpNamespaceId",
                table: "CsharpClasses",
                column: "CsharpNamespaceId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpClasses_ParentNodeId",
                table: "CsharpClasses",
                column: "ParentNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpEnumMembers_AstNodeId",
                table: "CsharpEnumMembers",
                column: "AstNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpEnumMembers_CsharpEnumId",
                table: "CsharpEnumMembers",
                column: "CsharpEnumId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpEnumMembers_ParentNodeId",
                table: "CsharpEnumMembers",
                column: "ParentNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpEnums_AstNodeId",
                table: "CsharpEnums",
                column: "AstNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpEnums_CsharpNamespaceId",
                table: "CsharpEnums",
                column: "CsharpNamespaceId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpEnums_ParentNodeId",
                table: "CsharpEnums",
                column: "ParentNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpEtcEntitys_AstNodeId",
                table: "CsharpEtcEntitys",
                column: "AstNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpEtcEntitys_ParentNodeId",
                table: "CsharpEtcEntitys",
                column: "ParentNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpMethods_AstNodeId",
                table: "CsharpMethods",
                column: "AstNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpMethods_ParentNodeId",
                table: "CsharpMethods",
                column: "ParentNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpNamespaces_AstNodeId",
                table: "CsharpNamespaces",
                column: "AstNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpNamespaces_ParentNodeId",
                table: "CsharpNamespaces",
                column: "ParentNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpStructs_AstNodeId",
                table: "CsharpStructs",
                column: "AstNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpStructs_CsharpNamespaceId",
                table: "CsharpStructs",
                column: "CsharpNamespaceId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpStructs_ParentNodeId",
                table: "CsharpStructs",
                column: "ParentNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpVariables_AstNodeId",
                table: "CsharpVariables",
                column: "AstNodeId");

            migrationBuilder.CreateIndex(
                name: "IX_CsharpVariables_ParentNodeId",
                table: "CsharpVariables",
                column: "ParentNodeId");
        }

        protected override void Down(MigrationBuilder migrationBuilder)
        {
            migrationBuilder.DropTable(
                name: "CsharpClasses");

            migrationBuilder.DropTable(
                name: "CsharpEnumMembers");

            migrationBuilder.DropTable(
                name: "CsharpEtcEntitys");

            migrationBuilder.DropTable(
                name: "CsharpMethods");

            migrationBuilder.DropTable(
                name: "CsharpStructs");

            migrationBuilder.DropTable(
                name: "CsharpVariables");

            migrationBuilder.DropTable(
                name: "CsharpEnums");

            migrationBuilder.DropTable(
                name: "CsharpNamespaces");

            migrationBuilder.DropTable(
                name: "CsharpAstNodes");
        }
    }
}
