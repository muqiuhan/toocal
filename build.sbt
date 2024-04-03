val scala3Version = "3.4.0"

lazy val root = project
  .in(file("."))
  .settings(
    name         := "toocal",
    version      := "0.0.1-SNAPSHOT",
    organization := "com.muqiuhan",
    assembly / test :=
      (Test / test).value,
    assembly / mainClass := Some("com.muqiuhan.Toocal"),
    scalaVersion         := scala3Version,
    libraryDependencies ++= Seq(
      "org.scalameta" %% "munit"  % "0.7.29" % Test,
      "com.outr"      %% "scribe" % "3.13.2"
    )
  )
