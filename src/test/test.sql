SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

DROP SCHEMA IF EXISTS `testdb` ;
CREATE SCHEMA IF NOT EXISTS `testdb` DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci ;
USE `testdb` ;

-- -----------------------------------------------------
-- Table `testdb`.`user`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `testdb`.`user` ;

CREATE TABLE IF NOT EXISTS `testdb`.`user` (
  `username` VARCHAR(16) NOT NULL,
  `email` VARCHAR(255) NULL,
  `create_time` TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,
  `idUser` INT NOT NULL AUTO_INCREMENT,
  UNIQUE INDEX `username_UNIQUE` (`username` ASC),
  PRIMARY KEY (`idUser`),
  UNIQUE INDEX `email_UNIQUE` (`email` ASC));


-- -----------------------------------------------------
-- Table `testdb`.`Company`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `testdb`.`Company` ;

CREATE TABLE IF NOT EXISTS `testdb`.`Company` (
  `idCompany` INT NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(45) NOT NULL,
  `user_idUser` INT NOT NULL,
  PRIMARY KEY (`idCompany`),
  INDEX `fk_Company_user1_idx` (`user_idUser` ASC),
  CONSTRAINT `fk_Company_user1`
    FOREIGN KEY (`user_idUser`)
    REFERENCES `testdb`.`user` (`idUser`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `testdb`.`Document`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `testdb`.`Document` ;

CREATE TABLE IF NOT EXISTS `testdb`.`Document` (
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

-- -----------------------------------------------------
-- Data for table `testdb`.`user`
-- -----------------------------------------------------
START TRANSACTION;
USE `testdb`;
INSERT INTO `testdb`.`user` (`username`, `email`, `create_time`, `idUser`) VALUES ('Loki', 'Loki@fakeEmail.com', '2013-11-19 14:34:21', 1);
INSERT INTO `testdb`.`user` (`username`, `email`, `create_time`, `idUser`) VALUES ('Hestia', 'Hestia@fakeEmail.com', '2013-11-19 14:34:21', 2);
INSERT INTO `testdb`.`user` (`username`, `email`, `create_time`, `idUser`) VALUES ('testClient', 'test@fakeEmail.com', '2013-11-19 14:34:21', 3);

COMMIT;


-- -----------------------------------------------------
-- Data for table `testdb`.`Company`
-- -----------------------------------------------------
START TRANSACTION;
USE `testdb`;
INSERT INTO `testdb`.`Company` (`idCompany`, `name`, `user_idUser`) VALUES (1, 'ING', 3);
INSERT INTO `testdb`.`Company` (`idCompany`, `name`, `user_idUser`) VALUES (2, 'ING', 2);
INSERT INTO `testdb`.`Company` (`idCompany`, `name`, `user_idUser`) VALUES (3, 'Loki inc.', 3);

COMMIT;


-- -----------------------------------------------------
-- Data for table `testdb`.`Document`
-- -----------------------------------------------------
START TRANSACTION;
USE `testdb`;
INSERT INTO `testdb`.`Document` (`idPage`, `created`, `user_idUser`, `Company_idCompany`, `content`) VALUES (1, '2013-11-19 14:34:21', 3, 1, 
NULL);
INSERT INTO `testdb`.`Document` (`idPage`, `created`, `user_idUser`, `Company_idCompany`, `content`) VALUES (2, '2013-11-19 14:34:21', 2, 2, 
NULL);
INSERT INTO `testdb`.`Document` (`idPage`, `created`, `user_idUser`, `Company_idCompany`, `content`) VALUES (3, '2013-11-19 14:34:22', 3, 3, 
NULL);

COMMIT;


