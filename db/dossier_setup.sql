SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

DROP SCHEMA IF EXISTS `mydb` ;
CREATE SCHEMA IF NOT EXISTS `mydb` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci ;
USE `mydb` ;

-- -----------------------------------------------------
-- Table `mydb`.`user`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`user` ;

CREATE TABLE IF NOT EXISTS `mydb`.`user` (
  `username` VARCHAR(16) NOT NULL,
  `email` VARCHAR(255) NULL,
  `create_time` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `idUser` INT NOT NULL AUTO_INCREMENT,
  UNIQUE INDEX `username_UNIQUE` (`username` ASC),
  PRIMARY KEY (`idUser`),
  UNIQUE INDEX `email_UNIQUE` (`email` ASC));


-- -----------------------------------------------------
-- Table `mydb`.`Company`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Company` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Company` (
  `idCompany` INT NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(45) NOT NULL,
  `user_idUser` INT NOT NULL,
  PRIMARY KEY (`idCompany`),
  INDEX `fk_Company_user1_idx` (`user_idUser` ASC),
  CONSTRAINT `fk_Company_user1`
    FOREIGN KEY (`user_idUser`)
    REFERENCES `mydb`.`user` (`idUser`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `mydb`.`Document`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `mydb`.`Document` ;

CREATE TABLE IF NOT EXISTS `mydb`.`Document` (
  `idPage` INT NOT NULL AUTO_INCREMENT,
  `created` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `user_idUser` INT NOT NULL,
  `Company_idCompany` INT NOT NULL,
  `content` LONGTEXT NULL,
  PRIMARY KEY (`idPage`),
  INDEX `fk_Document_user1_idx` (`user_idUser` ASC),
  INDEX `fk_Document_Company1_idx` (`Company_idCompany` ASC),
  FULLTEXT INDEX `ft_Document_content` (`content` ASC))
ENGINE = MyISAM;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
