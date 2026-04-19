using Microsoft.EntityFrameworkCore.Migrations;

#nullable disable

namespace Migrations.SqliteMigrations
{
  /// <inheritdoc />
  public partial class Initial : Migration
  {
    /// <inheritdoc />
    protected override void Up(MigrationBuilder migrationBuilder)
    {
      migrationBuilder.CreateTable(
          name: "CsharpAstNodes",
          columns: table => new
          {
            Id = table.Column<ulong>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            AstValue = table.Column<string>(type: "TEXT", nullable: true),
            AstSymbolType = table.Column<int>(type: "INTEGER", nullable: false),
            AstType = table.Column<int>(type: "INTEGER", nullable: false),
            Accessibility = table.Column<int>(type: "INTEGER", nullable: false),
            Location_range_start_line = table.Column<long>(type: "INTEGER", nullable: false),
            Location_range_start_column = table.Column<long>(type: "INTEGER", nullable: false),
            Location_range_end_line = table.Column<long>(type: "INTEGER", nullable: false),
            Location_range_end_column = table.Column<long>(type: "INTEGER", nullable: false),
            Path = table.Column<string>(type: "TEXT", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            RawKind = table.Column<ushort>(type: "INTEGER", nullable: false)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpAstNodes", x => x.Id);
          });

      migrationBuilder.CreateTable(
          name: "CsharpEtcEntitys",
          columns: table => new
          {
            Id = table.Column<long>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            EtcEntityType = table.Column<int>(type: "INTEGER", nullable: false),
            DeclaratorNodeId = table.Column<ulong>(type: "INTEGER", nullable: false),
            AstNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            ParentNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            Name = table.Column<string>(type: "TEXT", nullable: true),
            QualifiedName = table.Column<string>(type: "TEXT", nullable: true),
            DocumentationCommentXML = table.Column<string>(type: "TEXT", nullable: true),
            TypeHash = table.Column<long>(type: "INTEGER", nullable: false),
            QualifiedType = table.Column<string>(type: "TEXT", nullable: true)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpEtcEntitys", x => x.Id);
            table.ForeignKey(
                      name: "FK_CsharpEtcEntitys_CsharpAstNodes_AstNodeId",
                      column: x => x.AstNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpEtcEntitys_CsharpAstNodes_ParentNodeId",
                      column: x => x.ParentNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
          });

      migrationBuilder.CreateTable(
          name: "CsharpMethods",
          columns: table => new
          {
            Id = table.Column<long>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            MethodType = table.Column<int>(type: "INTEGER", nullable: false),
            AstNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            ParentNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            Name = table.Column<string>(type: "TEXT", nullable: true),
            QualifiedName = table.Column<string>(type: "TEXT", nullable: true),
            DocumentationCommentXML = table.Column<string>(type: "TEXT", nullable: true),
            TypeHash = table.Column<long>(type: "INTEGER", nullable: false),
            QualifiedType = table.Column<string>(type: "TEXT", nullable: true)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpMethods", x => x.Id);
            table.ForeignKey(
                      name: "FK_CsharpMethods_CsharpAstNodes_AstNodeId",
                      column: x => x.AstNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpMethods_CsharpAstNodes_ParentNodeId",
                      column: x => x.ParentNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
          });

      migrationBuilder.CreateTable(
          name: "CsharpNamespaces",
          columns: table => new
          {
            Id = table.Column<long>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            AstNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            ParentNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            Name = table.Column<string>(type: "TEXT", nullable: true),
            QualifiedName = table.Column<string>(type: "TEXT", nullable: true),
            DocumentationCommentXML = table.Column<string>(type: "TEXT", nullable: true)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpNamespaces", x => x.Id);
            table.ForeignKey(
                      name: "FK_CsharpNamespaces_CsharpAstNodes_AstNodeId",
                      column: x => x.AstNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpNamespaces_CsharpAstNodes_ParentNodeId",
                      column: x => x.ParentNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
          });

      migrationBuilder.CreateTable(
          name: "CsharpVariables",
          columns: table => new
          {
            Id = table.Column<long>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            VariableType = table.Column<int>(type: "INTEGER", nullable: false),
            AstNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            ParentNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            Name = table.Column<string>(type: "TEXT", nullable: true),
            QualifiedName = table.Column<string>(type: "TEXT", nullable: true),
            DocumentationCommentXML = table.Column<string>(type: "TEXT", nullable: true),
            TypeHash = table.Column<long>(type: "INTEGER", nullable: false),
            QualifiedType = table.Column<string>(type: "TEXT", nullable: true)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpVariables", x => x.Id);
            table.ForeignKey(
                      name: "FK_CsharpVariables_CsharpAstNodes_AstNodeId",
                      column: x => x.AstNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpVariables_CsharpAstNodes_ParentNodeId",
                      column: x => x.ParentNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
          });

      migrationBuilder.CreateTable(
          name: "CsharpClasses",
          columns: table => new
          {
            Id = table.Column<long>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            ClassType = table.Column<int>(type: "INTEGER", nullable: false),
            CsharpNamespaceId = table.Column<long>(type: "INTEGER", nullable: true),
            AstNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            ParentNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            Name = table.Column<string>(type: "TEXT", nullable: true),
            QualifiedName = table.Column<string>(type: "TEXT", nullable: true),
            DocumentationCommentXML = table.Column<string>(type: "TEXT", nullable: true)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpClasses", x => x.Id);
            table.ForeignKey(
                      name: "FK_CsharpClasses_CsharpAstNodes_AstNodeId",
                      column: x => x.AstNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpClasses_CsharpAstNodes_ParentNodeId",
                      column: x => x.ParentNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpClasses_CsharpNamespaces_CsharpNamespaceId",
                      column: x => x.CsharpNamespaceId,
                      principalTable: "CsharpNamespaces",
                      principalColumn: "Id");
          });

      migrationBuilder.CreateTable(
          name: "CsharpEnums",
          columns: table => new
          {
            Id = table.Column<long>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            CsharpNamespaceId = table.Column<long>(type: "INTEGER", nullable: true),
            AstNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            ParentNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            Name = table.Column<string>(type: "TEXT", nullable: true),
            QualifiedName = table.Column<string>(type: "TEXT", nullable: true),
            DocumentationCommentXML = table.Column<string>(type: "TEXT", nullable: true)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpEnums", x => x.Id);
            table.ForeignKey(
                      name: "FK_CsharpEnums_CsharpAstNodes_AstNodeId",
                      column: x => x.AstNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpEnums_CsharpAstNodes_ParentNodeId",
                      column: x => x.ParentNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpEnums_CsharpNamespaces_CsharpNamespaceId",
                      column: x => x.CsharpNamespaceId,
                      principalTable: "CsharpNamespaces",
                      principalColumn: "Id");
          });

      migrationBuilder.CreateTable(
          name: "CsharpStructs",
          columns: table => new
          {
            Id = table.Column<long>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            CsharpNamespaceId = table.Column<long>(type: "INTEGER", nullable: true),
            AstNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            ParentNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            Name = table.Column<string>(type: "TEXT", nullable: true),
            QualifiedName = table.Column<string>(type: "TEXT", nullable: true),
            DocumentationCommentXML = table.Column<string>(type: "TEXT", nullable: true)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpStructs", x => x.Id);
            table.ForeignKey(
                      name: "FK_CsharpStructs_CsharpAstNodes_AstNodeId",
                      column: x => x.AstNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpStructs_CsharpAstNodes_ParentNodeId",
                      column: x => x.ParentNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpStructs_CsharpNamespaces_CsharpNamespaceId",
                      column: x => x.CsharpNamespaceId,
                      principalTable: "CsharpNamespaces",
                      principalColumn: "Id");
          });

      migrationBuilder.CreateTable(
          name: "CsharpEnumMembers",
          columns: table => new
          {
            Id = table.Column<long>(type: "INTEGER", nullable: false)
                  .Annotation("Sqlite:Autoincrement", true),
            EqualsValue = table.Column<int>(type: "INTEGER", nullable: false),
            CsharpEnumId = table.Column<long>(type: "INTEGER", nullable: true),
            AstNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            ParentNodeId = table.Column<ulong>(type: "INTEGER", nullable: true),
            EntityHash = table.Column<long>(type: "INTEGER", nullable: false),
            Name = table.Column<string>(type: "TEXT", nullable: true),
            QualifiedName = table.Column<string>(type: "TEXT", nullable: true),
            DocumentationCommentXML = table.Column<string>(type: "TEXT", nullable: true)
          },
          constraints: table =>
          {
            table.PrimaryKey("PK_CsharpEnumMembers", x => x.Id);
            table.ForeignKey(
                      name: "FK_CsharpEnumMembers_CsharpAstNodes_AstNodeId",
                      column: x => x.AstNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpEnumMembers_CsharpAstNodes_ParentNodeId",
                      column: x => x.ParentNodeId,
                      principalTable: "CsharpAstNodes",
                      principalColumn: "Id");
            table.ForeignKey(
                      name: "FK_CsharpEnumMembers_CsharpEnums_CsharpEnumId",
                      column: x => x.CsharpEnumId,
                      principalTable: "CsharpEnums",
                      principalColumn: "Id");
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

    /// <inheritdoc />
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
